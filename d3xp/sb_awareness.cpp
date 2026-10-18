#include "sys/platform.h"

#include "Game_local.h"
#include "Player.h"
#include "sb_record.h"
#include "sb_awareness.h"

CLASS_DECLARATION( idEntity, idStBrielleAwarenessDirector )
END_CLASS

idStBrielleAwarenessDirector::idStBrielleAwarenessDirector() {
	lastTier = -1;
	nextCheckTime = 0;
	checkIntervalMS = 250;
	debug = false;

	annexLedgerWasInside = false;
	annexCorrectionApplied = false;
	annexPhotoConfigured = false;
	annexPhonePulseBright = false;
	annexPhoneNextPulseTime = 0;

	annexFiledPersonActive = false;
	annexFiledPersonNextMoveTime = 0;
	annexFiledPersonObservedSince = 0;
}

void idStBrielleAwarenessDirector::Spawn( void ) {
	idStBrielleRecord::EnsureDefaults();

	checkIntervalMS = spawnArgs.GetInt( "check_interval_ms", "250" );
	if ( checkIntervalMS < 50 ) {
		checkIntervalMS = 50;
	}

	debug = spawnArgs.GetBool( "awareness_debug", "0" );

	if ( spawnArgs.GetBool( "fire_current_on_spawn", "1" ) ) {
		lastTier = -1;
	} else {
		lastTier = static_cast<int>( idStBrielleRecord::GetAwarenessTier() );
	}

	annexLedgerWasInside = false;
	annexCorrectionApplied = false;
	annexPhotoConfigured = false;
	annexPhonePulseBright = false;
	annexPhoneNextPulseTime = gameLocal.time;

	annexFiledPersonActive = false;
	annexFiledPersonNextMoveTime = gameLocal.time;
	annexFiledPersonObservedSince = 0;

	nextCheckTime = gameLocal.time;
	BecomeActive( TH_THINK );
}

void idStBrielleAwarenessDirector::Think( void ) {
	if ( gameLocal.time < nextCheckTime ) {
		return;
	}

	nextCheckTime = gameLocal.time + checkIntervalMS;

	// The Annex vertical slice uses this already-present director for its quiet
	// post-214 correction. Keeping it here avoids another always-thinking map
	// entity and lets Awareness remain invisible to the player.
	UpdateAnnexSequence();

	const int tier = static_cast<int>( idStBrielleRecord::GetAwarenessTier() );
	if ( tier == lastTier ) {
		return;
	}

	if ( debug ) {
		gameLocal.Printf(
			"[ST. BRIELLE AWARENESS] %s -> %s (value %d)\n",
			lastTier < 0 ? "uninitialized" :
				idStBrielleRecord::GetAwarenessTierName(
					static_cast<idStBrielleRecord::awarenessTier_t>( lastTier ) ),
			idStBrielleRecord::GetAwarenessTierName(
				static_cast<idStBrielleRecord::awarenessTier_t>( tier ) ),
			idStBrielleRecord::GetAwareness()
		);
	}

	lastTier = tier;
	ActivateTierTargets( tier );
}

void idStBrielleAwarenessDirector::ActivateTierTargets( int tier ) {
	const idStr prefix = va( "tier%d_target", tier );
	const idKeyValue *kv = spawnArgs.MatchPrefix( prefix.c_str() );

	while ( kv ) {
		const char *entityName = kv->GetValue().c_str();
		idEntity *ent = gameLocal.FindEntity( entityName );

		if ( !ent ) {
			gameLocal.Warning(
				"idStBrielleAwarenessDirector '%s' could not find tier target '%s'.",
				GetName(), entityName
			);
		} else if ( ent->RespondsTo( EV_Activate ) || ent->HasSignal( SIG_TRIGGER ) ) {
			ent->Signal( SIG_TRIGGER );
			ent->ProcessEvent( &EV_Activate, this );
		}

		kv = spawnArgs.MatchPrefix( prefix.c_str(), kv );
	}
}

void idStBrielleAwarenessDirector::ConfigureNotice( idEntity *notice,
	const char *kicker, const char *title,
	const char *line1, const char *line2, const char *line3, const char *line4,
	const char *footer, const char *duration ) {
	if ( !notice ) {
		return;
	}

	notice->spawnArgs.Set( "notice_kicker", kicker ? kicker : "" );
	notice->spawnArgs.Set( "notice_title", title ? title : "" );
	notice->spawnArgs.Set( "notice_line1", line1 ? line1 : "" );
	notice->spawnArgs.Set( "notice_line2", line2 ? line2 : "" );
	notice->spawnArgs.Set( "notice_line3", line3 ? line3 : "" );
	notice->spawnArgs.Set( "notice_line4", line4 ? line4 : "" );
	notice->spawnArgs.Set( "notice_footer", footer ? footer : "" );
	notice->spawnArgs.Set( "notice_duration", duration ? duration : "6.0" );
}

void idStBrielleAwarenessDirector::ConfigureInteractable( idEntity *ent,
	const char *newDisplayName, const char *action, const char *targetName, bool useOnce ) {
	if ( !ent ) {
		return;
	}

	ent->spawnArgs.Set( "displayname", newDisplayName ? newDisplayName : "" );
	ent->spawnArgs.Set( "stbrielle_action", action ? action : "INTERACT" );
	ent->spawnArgs.Set( "stbrielle_interact", "1" );
	ent->spawnArgs.Set( "stbrielle_activate_targets", "1" );
	ent->spawnArgs.Set( "stbrielle_use_once", useOnce ? "1" : "0" );
	ent->spawnArgs.Set( "stbrielle_interact_debug", "1" );
	ent->spawnArgs.Set( "target", targetName ? targetName : "" );
	ent->displayName = newDisplayName ? newDisplayName : "";

	// Runtime retargeting is used by the Annex telephone and continuity exit.
	// Clear mapper-authored secondary targets so a corrected interaction cannot
	// accidentally fire both its old and new administrative state.
	for ( int i = 1; i < 16; ++i ) {
		ent->spawnArgs.Delete( va( "target%d", i ) );
	}
	ent->targets.Clear();

	// These decorative statics already have compiled clip models; several were
	// intentionally non-solid for dressing. A thin notice board can safely be
	// made traceable without affecting navigation in a meaningful way.
	if ( !idStr::Icmp( ent->GetName(), "annex_notice_board" ) ) {
		ent->GetPhysics()->SetContents( CONTENTS_SOLID );
	}

	ent->FindTargets();
}

void idStBrielleAwarenessDirector::ConfigureAnnexPhoto( bool corrected ) {
	idEntity *photo = gameLocal.FindEntity( "annex_notice_board" );
	idEntity *notice = gameLocal.FindEntity( "annex_workorder_notice" );
	if ( !photo || !notice ) {
		return;
	}

	ConfigureInteractable( photo, "MUNICIPAL ANNEX STAFF PHOTOGRAPH", "INSPECT", "annex_workorder_notice", false );

	if ( corrected ) {
		ConfigureNotice(
			notice,
			"OFFICE OF PERPETUAL RECORD / PERSONNEL",
			"MUNICIPAL ANNEX STAFF / 1987",
			"IMAGE COUNT: SIX",
			"UNLISTED INDIVIDUAL: $PLAYER",
			"ROOM 214: ASSIGNED / PRESENT",
			"APPOINTMENT TIME: 23:44",
			"CITY COPY / PHOTOGRAPH 6-B / CORRECTED",
			"8.0"
		);
	} else {
		ConfigureNotice(
			notice,
			"OFFICE OF PERPETUAL RECORD / PERSONNEL",
			"MUNICIPAL ANNEX STAFF / 1987",
			"IMAGE COUNT: FIVE",
			"ARCHIVES / CONTINUITY / BUILDING SERVICES",
			"ROOM 214: NO ASSIGNMENT",
			"NEGATIVE FILE: VERIFIED",
			"CITY COPY / PHOTOGRAPH 6-B",
			"7.0"
		);
	}

	annexPhotoConfigured = true;
}

void idStBrielleAwarenessDirector::ConfigureAnnexPhone( void ) {
	// The reception countertop is the solid interaction surface directly under
	// the modeled desk telephone in annex_dressing_pass_03. Using it keeps this
	// prototype traceable without adding another BSP brush entity.
	idEntity *phoneSurface = gameLocal.FindEntity( "annex_reception_countertop" );
	idEntity *notice = gameLocal.FindEntity( "annex_breaker_notice" );
	if ( !phoneSurface || !notice ) {
		return;
	}

	const bool phoneAnswered = idStBrielleRecord::GetBool( "sb_annex_phone_answered", false );
	phoneSurface->spawnArgs.Set( "snd_annex_phone_ring", "snd_stbrielle_annex_phone_ring" );

	// A persisted answer must win over presentation state restored from either
	// a savegame or a fresh map load. Never let an already-answered phone ring
	// again simply because the Annex correction is being reconstructed.
	if ( phoneAnswered ) {
		phoneSurface->StopSound( SND_CHANNEL_BODY, false );
		phoneSurface->spawnArgs.Set( "stbrielle_interact", "0" );
		return;
	}

	ConfigureInteractable( phoneSurface, "RECEPTION TELEPHONE", "ANSWER", "annex_breaker_notice", true );
	ConfigureNotice(
		notice,
		"MUNICIPAL VOICE SERVICE / INTERNAL",
		"CALL CONNECTED / LINE 04-214",
		"CALLER: ROOM 214",
		"VOICE MATCH: $PLAYER",
		"PLACED: 23:44 / RECEIVED: 23:44",
		"TRANSCRIPT: YOU LEFT BEFORE YOU ARRIVED.",
		"LINE PHYSICALLY DISCONNECTED / 1971",
		"8.0"
	);

	phoneSurface->StartSound( "snd_annex_phone_ring", SND_CHANNEL_BODY, 0, false, NULL );
}

void idStBrielleAwarenessDirector::ConfigureAnnexExit( bool reviewed ) {
	idEntity *exitTerminal = gameLocal.FindEntity( "annex_exit_terminal" );
	idEntity *notice = gameLocal.FindEntity( "annex_exit_complete_notice" );
	if ( !exitTerminal || !notice ) {
		return;
	}

	// The map's existing RETURN target ultimately writes sb_annex_complete.
	// Treat that record as the authoritative terminal state so a save/load or
	// fresh map entry cannot reopen a correction the player already filed.
	if ( idStBrielleRecord::GetBool( "sb_annex_complete", false ) ) {
		ConfigureInteractable( exitTerminal, "CONTINUITY EXIT / FILED", "FILED", "", true );
		exitTerminal->spawnArgs.Set( "stbrielle_interact", "0" );
		exitTerminal->targets.Clear();
		ConfigureNotice(
			notice,
			"MUNICIPAL CONTINUITY / EXIT CONTROL",
			"RETURN FILED",
			"NAME: $PLAYER",
			"PRIOR DEPARTURE: 23:44",
			"RETURN: 23:44",
			"CONTINUITY HOLD: RELEASED",
			"YOU MAY PROCEED",
			"8.0"
		);
		return;
	}

	if ( reviewed ) {
		// RETURN is deliberately one-use. Player.cpp disables the interaction as
		// soon as it succeeds, while the existing map target records completion.
		ConfigureInteractable( exitTerminal, "CONTINUITY EXIT", "RETURN", "annex_exit_ready_check", true );
		ConfigureNotice(
			notice,
			"MUNICIPAL CONTINUITY / EXIT CONTROL",
			"DEPARTURE CORRECTION ACCEPTED",
			"NAME: $PLAYER",
			"PRIOR DEPARTURE: 23:44",
			"CURRENT STATUS: PRESENT",
			"ACTION: RETURN TO FILE",
			"ONE RETURN AUTHORIZED",
			"8.0"
		);
	} else {
		ConfigureInteractable( exitTerminal, "CONTINUITY EXIT / CLOSED", "REVIEW", "annex_exit_complete_notice", true );
		ConfigureNotice(
			notice,
			"MUNICIPAL CONTINUITY / EXIT CONTROL",
			"DEPARTURE ALREADY FILED",
			"NAME: $PLAYER",
			"DEPARTURE: 23:44",
			"CURRENT STATUS: PRESENT",
			"SECOND DEPARTURE: NOT PERMITTED",
			"YOU MAY NOT LEAVE TWICE",
			"8.0"
		);
	}
}

void idStBrielleAwarenessDirector::ApplyAnnexCorrection( void ) {
	if ( annexCorrectionApplied ) {
		return;
	}

	// Shift one repeated arch by just over a foot. Nothing announces this; the
	// corridor simply stops matching the rhythm the player walked through.
	idEntity *arch = gameLocal.FindEntity( "annex_gate_arch_b" );
	if ( arch ) {
		arch->SetOrigin( idVec3( 36.0f, 0.0f, 0.0f ) );
	}

	// Remove one pool from the ceiling cadence so the changed interval is also
	// registered subconsciously in light rather than through an explicit cue.
	idEntity *ceilingPool = gameLocal.FindEntity( "annex_ceiling_pool_3" );
	if ( ceilingPool && ceilingPool->RespondsTo( EV_Activate ) ) {
		ceilingPool->ProcessEvent( &EV_Activate, this );
	}

	// The old 214 paint scar becomes a dark extra shape on the personnel board:
	// an intentionally crude first visual stand-in for the sixth person that was
	// not present in the photograph before the room was serviced.
	idEntity *photoCorrection = gameLocal.FindEntity( "annex_room214_absence_trace" );
	if ( photoCorrection ) {
		photoCorrection->SetOrigin( idVec3( -672.0f, -75.0f, -24.0f ) );
		photoCorrection->Show();
	}

	// Room 214 looked like an ordinary Annex room on first entry. Once the player
	// crosses back through the impossible doorway, the Record quietly replaces
	// its surfaces with a corrected version: grout lines drift, erased marks sit
	// under the plaster, wood grain changes direction, and steel scratches stop
	// agreeing with the light. The geometry and collision remain untouched.
	const idDeclSkin *room214CorrectedSkin = declManager->FindSkin( "stbrielle/room214_corrected", false );
	if ( room214CorrectedSkin ) {
		const char *room214Visuals[] = {
			"annex_room214_contents",
			"annex_room214_reveal_frame",
			"annex_214_ledger"
		};
		for ( int i = 0; i < 3; ++i ) {
			idEntity *room214Entity = gameLocal.FindEntity( room214Visuals[i] );
			if ( room214Entity ) {
				room214Entity->SetSkin( room214CorrectedSkin );
				room214Entity->UpdateVisuals();
			}
		}
	} else if ( debug ) {
		gameLocal.Warning( "[ST. BRIELLE ANNEX] Room 214 corrected skin was not found." );
	}

	// The corrected threshold is physically traversable, but it no longer quite
	// agrees with the hole in the wall. Shift only the non-solid reveal frame so
	// player collision remains stable. A few units are enough to make the top and
	// jamb registration feel subtly impossible when the player turns back.
	idEntity *room214Frame = gameLocal.FindEntity( "annex_room214_reveal_frame" );
	if ( room214Frame ) {
		const idVec3 frameOrigin = room214Frame->GetPhysics()->GetOrigin();
		room214Frame->SetOrigin( frameOrigin + idVec3( -2.0f, 3.0f, -3.5f ) );
		room214Frame->UpdateVisuals();
	}

	// Lighting stops agreeing on where the center of the opening is. The main
	// reveal light drifts corridor-left while the two internal sources split in
	// opposite directions, making the threshold read narrower from one angle and
	// taller from another without touching the BSP opening itself.
	idEntity *room214RevealLight = gameLocal.FindEntity( "annex_room214_light" );
	if ( room214RevealLight ) {
		room214RevealLight->SetOrigin( idVec3( 732.0f, 251.0f, 174.0f ) );
		room214RevealLight->SetColor( 0.17f, 0.24f, 0.34f );
	}

	idEntity *room214LightA = gameLocal.FindEntity( "annex_214_inner_light_a" );
	if ( room214LightA ) {
		room214LightA->SetOrigin( idVec3( 748.0f, 247.0f, 226.0f ) );
		room214LightA->SetColor( 0.38f, 0.45f, 0.52f );

		// Anchor the threshold sound to a real in-room light entity so the engine's
		// normal distance attenuation places the pressure condition at the doorway.
		room214LightA->spawnArgs.Set( "snd_room214_threshold", "snd_stbrielle_room214_threshold" );
		if ( !room214LightA->IsPlayingSound( SND_CHANNEL_BODY ) ) {
			room214LightA->StartSound( "snd_room214_threshold", SND_CHANNEL_BODY, 0, false, NULL );
		}
	}

	idEntity *room214LightB = gameLocal.FindEntity( "annex_214_inner_light_b" );
	if ( room214LightB ) {
		room214LightB->SetOrigin( idVec3( 866.0f, 277.0f, 118.0f ) );
		room214LightB->SetColor( 0.22f, 0.018f, 0.034f );
	}

	ConfigureAnnexPhoto( true );
	ConfigureAnnexPhone();

	const bool phoneAnswered = idStBrielleRecord::GetBool( "sb_annex_phone_answered", false );

	// Once the correction is visible, the old mapper-authored exit must no longer
	// be a bypass around the ringing phone. Hold it inert until the receiver is
	// answered; ConfigureAnnexExit() will rebuild REVIEW and RETURN afterward.
	if ( !phoneAnswered ) {
		idEntity *exitTerminal = gameLocal.FindEntity( "annex_exit_terminal" );
		if ( exitTerminal ) {
			ConfigureInteractable( exitTerminal, "CONTINUITY EXIT / HOLD", "HOLD", "", true );
			exitTerminal->spawnArgs.Set( "stbrielle_interact", "0" );
			exitTerminal->spawnArgs.Set( "stbrielle_activate_targets", "0" );
			exitTerminal->targets.Clear();
		}
	}

	idEntity *phoneLight = gameLocal.FindEntity( "annex_workorder_light" );
	if ( phoneLight ) {
		phoneLight->SetOrigin( idVec3( -420.0f, 64.0f, 92.0f ) );
		if ( phoneAnswered ) {
			phoneLight->SetColor( 0.07f, 0.025f, 0.018f );
		} else {
			phoneLight->SetColor( 0.86f, 0.22f, 0.055f );
		}
	}

	annexPhonePulseBright = !phoneAnswered;
	annexPhoneNextPulseTime = gameLocal.time + 450;
	annexCorrectionApplied = true;

	if ( debug ) {
		gameLocal.Printf( "[ST. BRIELLE ANNEX] post-214 continuity correction applied\n" );
	}
}

void idStBrielleAwarenessDirector::UpdateAnnexFiledPerson( idPlayer *player ) {
	if ( !player ) {
		return;
	}

	idEntity *filedPerson = gameLocal.FindEntity( "annex_room214_absence_trace" );
	if ( !filedPerson ) {
		return;
	}

	const bool resolved = idStBrielleRecord::GetBool( "sb_annex_filed_person_resolved", false );
	if ( resolved ) {
		filedPerson->StopSound( SND_CHANNEL_BODY, false );
		filedPerson->Hide();
		annexFiledPersonActive = false;
		return;
	}

	if ( !idStBrielleRecord::GetBool( "sb_annex_complete", false ) ) {
		return;
	}

	// The source brush was authored as a flat sixth silhouette over the personnel
	// photograph. Its brush coordinates are baked around this center, so runtime
	// movement works by positioning the entity origin relative to that visual
	// center rather than pretending its physics origin is the figure itself.
	const idVec3 visualOffset( 539.0f, 260.0f, 174.0f );

	if ( !annexFiledPersonActive ) {
		annexFiledPersonActive = true;
		annexFiledPersonNextMoveTime = gameLocal.time + 1100;
		annexFiledPersonObservedSince = 0;

		// It does not spawn out of thin air. The sixth shape simply stops being on
		// the photograph and is next seen in the central corridor.
		const idVec3 startCenter( 300.0f, 0.0f, 82.0f );
		filedPerson->SetOrigin( startCenter - visualOffset );
		filedPerson->Show();
		filedPerson->UpdateVisuals();
		filedPerson->spawnArgs.Set( "snd_filed_person", "snd_stbrielle_annex_filed_person" );
		filedPerson->StartSound( "snd_filed_person", SND_CHANNEL_BODY, 0, false, NULL );

		idEntity *notice = gameLocal.FindEntity( "annex_exit_complete_notice" );
		if ( notice ) {
			ConfigureNotice(
				notice,
				"OFFICE OF PERPETUAL RECORD / PERSONNEL",
				"PHOTOGRAPH 6-B / LOCATION EXCEPTION",
				"IMAGE COUNT: SIX",
				"IMAGE SIX: NOT IN FRAME",
				"CURRENT LOCATION: MUNICIPAL ANNEX",
				"SUBJECT STATUS: APPROACHING",
				"DO NOT CREATE A SECOND COPY",
				"7.0"
			);
			notice->ProcessEvent( &EV_Activate, player );
		}

		if ( debug ) {
			gameLocal.Printf( "[ST. BRIELLE ANNEX] personnel image six detached; first hostile active\n" );
		}
	}

	const idVec3 playerOrigin = player->GetPhysics()->GetOrigin();
	idVec3 filedCenter = filedPerson->GetPhysics()->GetOrigin() + visualOffset;
	idVec3 toPlayer = playerOrigin + idVec3( 0.0f, 0.0f, 54.0f ) - filedCenter;
	const float contactDistance = toPlayer.Length();

	// Contact is a bureaucratic consequence rather than a conventional monster
	// attack. The player remains alive, but St. Brielle remembers that the
	// duplicate image reached them.
	if ( contactDistance < 74.0f ) {
		idStBrielleRecord::SetBool( "sb_annex_filed_person_contact", true );
		idStBrielleRecord::AddInt( "sb_annex_filed_person_contact_count", 1 );
		idStBrielleRecord::AddAwareness( 4 );

		int stability = idStBrielleRecord::GetInt( "sb_record_stability", 100 ) - 15;
		if ( stability < 0 ) {
			stability = 0;
		}
		idStBrielleRecord::SetInt( "sb_record_stability", stability );

		idEntity *notice = gameLocal.FindEntity( "annex_exit_complete_notice" );
		if ( notice ) {
			ConfigureNotice(
				notice,
				"OFFICE OF PERPETUAL RECORD / PERSONNEL",
				"CONTACT RECORDED",
				"SUBJECT: $PLAYER",
				"DUPLICATE PRESENCE: VERIFIED",
				"RECORD STABILITY: REDUCED",
				"CORRECTION REMAINS ACTIVE",
				"THIS CONTACT WILL BE RETAINED",
				"7.0"
			);
			notice->ProcessEvent( &EV_Activate, player );
		}

		// Give the player another chance to learn the rule. The figure withdraws
		// several meters, but the persistent consequence is not undone.
		float resetX = playerOrigin.x < 0.0f ? playerOrigin.x + 360.0f : playerOrigin.x - 360.0f;
		resetX = idMath::ClampFloat( -620.0f, 620.0f, resetX );
		filedCenter.Set( resetX, 0.0f, 82.0f );
		filedPerson->SetOrigin( filedCenter - visualOffset );
		filedPerson->UpdateVisuals();
		annexFiledPersonObservedSince = 0;
		annexFiledPersonNextMoveTime = gameLocal.time + 2200;
		return;
	}

	renderView_t *view = player->GetRenderView();
	bool observed = false;
	if ( view ) {
		idVec3 toFiledPerson = filedCenter - view->vieworg;
		const float distance = toFiledPerson.Normalize();
		const float facing = view->viewaxis[0] * toFiledPerson;

		// A generous cone makes the rule legible in first person: it must actually
		// be in front of the player, not merely somewhere on-screen at the edge.
		observed = distance < 1000.0f && facing > 0.84f;
	}

	if ( observed ) {
		if ( annexFiledPersonObservedSince <= 0 ) {
			annexFiledPersonObservedSince = gameLocal.time;
		}

		if ( gameLocal.time - annexFiledPersonObservedSince >= 2600 ) {
			idStBrielleRecord::SetBool( "sb_annex_filed_person_resolved", true );
			idStBrielleRecord::SetBool( "sb_annex_first_hostile_survived", true );
			idStBrielleRecord::AddAwareness( 1 );
			filedPerson->StopSound( SND_CHANNEL_BODY, false );
			filedPerson->Hide();
			annexFiledPersonActive = false;

			idEntity *notice = gameLocal.FindEntity( "annex_exit_complete_notice" );
			if ( notice ) {
				ConfigureNotice(
					notice,
					"OFFICE OF PERPETUAL RECORD / PERSONNEL",
					"DUPLICATE IMAGE WITHDRAWN",
					"PHOTOGRAPH 6-B: FIVE SUBJECTS",
					"OBSERVATION: ACCEPTED",
					"SECOND PRESENCE: NOT RETAINED",
					"CURRENT STATUS: SINGLE",
					"YOU MAY CONTINUE",
					"8.0"
				);
				notice->ProcessEvent( &EV_Activate, player );
			}

			if ( debug ) {
				gameLocal.Printf( "[ST. BRIELLE ANNEX] personnel image six stabilized and withdrawn\n" );
			}
		}
		return;
	}

	annexFiledPersonObservedSince = 0;

	if ( gameLocal.time < annexFiledPersonNextMoveTime ) {
		return;
	}

	// The figure advances in discontinuous corrections rather than walking. It
	// remains in the main hall even if the player ducks into a side office, which
	// keeps this prototype from clipping straight through Annex walls.
	idVec3 target = playerOrigin + idVec3( 0.0f, 0.0f, 54.0f );
	target.y = idMath::ClampFloat( -72.0f, 72.0f, target.y );
	target.z = 82.0f;

	idVec3 advance = target - filedCenter;
	advance.z = 0.0f;
	const float remaining = advance.Normalize();
	if ( remaining > 1.0f ) {
		const float step = idStBrielleRecord::GetAwareness() >= 10 ? 86.0f : 68.0f;
		const float moveDistance = remaining < step ? remaining : step;
		filedCenter += advance * moveDistance;
		filedCenter.x = idMath::ClampFloat( -760.0f, 760.0f, filedCenter.x );
		filedCenter.y = idMath::ClampFloat( -72.0f, 72.0f, filedCenter.y );
		filedCenter.z = 82.0f;
		filedPerson->SetOrigin( filedCenter - visualOffset );
		filedPerson->UpdateVisuals();
	}

	annexFiledPersonNextMoveTime = gameLocal.time + 700;
}

void idStBrielleAwarenessDirector::UpdateAnnexSequence( void ) {
	// Presence of this entity is our cheap map identity check; no Annex-specific
	// behavior runs on other St. Brielle maps.
	if ( !gameLocal.FindEntity( "annex_room214_check" ) ) {
		return;
	}

	const bool workOrderRead = idStBrielleRecord::GetBool( "sb_annex_workorder_read", false );
	const bool ledgerRead = idStBrielleRecord::GetBool( "sb_annex_214_ledger_read", false );
	const bool corrected = idStBrielleRecord::GetBool( "sb_annex_after_214", false );

	// Once the work order has been read, its already-fired notice target is free
	// to become the inspection text for the personnel photograph.
	if ( workOrderRead && !corrected && !annexPhotoConfigured ) {
		ConfigureAnnexPhoto( false );
	}

	if ( corrected && !annexCorrectionApplied ) {
		ApplyAnnexCorrection();
	}

	idPlayer *player = gameLocal.GetLocalPlayer();
	if ( !corrected && ledgerRead && player ) {
		const idVec3 &p = player->GetPhysics()->GetOrigin();

		// Arm only after the player has physically occupied Room 214. That keeps a
		// persistent ledger flag from causing the correction at the map entrance.
		if ( p.x > 600.0f && p.y > 180.0f && p.y < 340.0f ) {
			annexLedgerWasInside = true;
		}

		// Crossing back through the impossible doorway performs the correction.
		if ( annexLedgerWasInside && p.x < 548.0f ) {
			idStBrielleRecord::SetBool( "sb_annex_after_214", true );
			ApplyAnnexCorrection();
		}
	}

	if ( !annexCorrectionApplied ) {
		return;
	}

	idEntity *phoneSurface = gameLocal.FindEntity( "annex_reception_countertop" );
	const bool phoneAnswered = idStBrielleRecord::GetBool( "sb_annex_phone_answered", false );

	// stbrielle_use_once disables stbrielle_interact after a successful frob.
	// That gives the director a clean, generic way to notice the receiver being
	// answered without hard-coding a second interaction path into Player.cpp.
	if ( !phoneAnswered && phoneSurface && !phoneSurface->spawnArgs.GetBool( "stbrielle_interact", "0" ) ) {
		idStBrielleRecord::SetBool( "sb_annex_phone_answered", true );
		phoneSurface->StopSound( SND_CHANNEL_BODY, false );

		idEntity *phoneLight = gameLocal.FindEntity( "annex_workorder_light" );
		if ( phoneLight ) {
			phoneLight->SetColor( 0.07f, 0.025f, 0.018f );
		}

		// The call does not end the beat. It invalidates the ordinary exit because
		// the city already has a 23:44 departure on file for a person still here.
		ConfigureAnnexExit( false );

		if ( debug ) {
			gameLocal.Printf( "[ST. BRIELLE ANNEX] reception telephone answered; departure record now conflicts\n" );
		}
		return;
	}

	if ( !phoneAnswered && gameLocal.time >= annexPhoneNextPulseTime ) {
		annexPhonePulseBright = !annexPhonePulseBright;
		annexPhoneNextPulseTime = gameLocal.time + ( annexPhonePulseBright ? 420 : 240 );

		idEntity *phoneLight = gameLocal.FindEntity( "annex_workorder_light" );
		if ( phoneLight ) {
			if ( annexPhonePulseBright ) {
				phoneLight->SetColor( 0.86f, 0.22f, 0.055f );
			} else {
				phoneLight->SetColor( 0.12f, 0.035f, 0.020f );
			}
		}
	}

	if ( phoneAnswered ) {
		if ( phoneSurface ) {
			phoneSurface->StopSound( SND_CHANNEL_BODY, false );
		}

		idEntity *exitTerminal = gameLocal.FindEntity( "annex_exit_terminal" );
		if ( !exitTerminal ) {
			return;
		}

		const bool exitReviewed = idStBrielleRecord::GetBool( "sb_annex_exit_reviewed", false );
		const bool exitComplete = idStBrielleRecord::GetBool( "sb_annex_complete", false );
		const bool exitInteractive = exitTerminal->spawnArgs.GetBool( "stbrielle_interact", "0" );
		const char *exitAction = exitTerminal->spawnArgs.GetString( "stbrielle_action", "" );

		if ( exitComplete ) {
			if ( idStr::Icmp( exitAction, "FILED" ) != 0 || exitInteractive ) {
				ConfigureAnnexExit( true );
				if ( debug ) {
					gameLocal.Printf( "[ST. BRIELLE ANNEX] continuity return filed; exit correction complete\n" );
				}
			}

			// The FILED state opens the first hostile beat. Photograph 6-B's sixth
			// subject is no longer a passive correction once the player's duplicate
			// departure has been accepted into the city record.
			UpdateAnnexFiledPerson( player );
			return;
		}

		if ( !exitReviewed ) {
			// Fresh map loads rebuild the phone state from the persistent record. Put
			// the terminal back into REVIEW without erasing the one-use disable that
			// tells us the player actually reviewed the contradiction.
			if ( idStr::Icmp( exitAction, "REVIEW" ) != 0 ) {
				ConfigureAnnexExit( false );
			}

			if ( !exitTerminal->spawnArgs.GetBool( "stbrielle_interact", "0" ) ) {
				idStBrielleRecord::SetBool( "sb_annex_exit_reviewed", true );
				ConfigureAnnexExit( true );

				if ( debug ) {
					gameLocal.Printf( "[ST. BRIELLE ANNEX] duplicate departure reviewed; one return authorized\n" );
				}
			}
		} else if ( idStr::Icmp( exitAction, "RETURN" ) != 0 ) {
			ConfigureAnnexExit( true );
		}
	}
}

void idStBrielleAwarenessDirector::Save( idSaveGame *savefile ) const {
	savefile->WriteInt( lastTier );
	savefile->WriteInt( nextCheckTime );
	savefile->WriteInt( checkIntervalMS );
	savefile->WriteBool( debug );

	savefile->WriteBool( annexLedgerWasInside );
	savefile->WriteBool( annexCorrectionApplied );
	savefile->WriteBool( annexPhotoConfigured );
	savefile->WriteBool( annexPhonePulseBright );
	savefile->WriteInt( annexPhoneNextPulseTime );

	savefile->WriteBool( annexFiledPersonActive );
	savefile->WriteInt( annexFiledPersonNextMoveTime );
	savefile->WriteInt( annexFiledPersonObservedSince );
}

void idStBrielleAwarenessDirector::Restore( idRestoreGame *savefile ) {
	savefile->ReadInt( lastTier );
	savefile->ReadInt( nextCheckTime );
	savefile->ReadInt( checkIntervalMS );
	savefile->ReadBool( debug );

	savefile->ReadBool( annexLedgerWasInside );
	savefile->ReadBool( annexCorrectionApplied );
	savefile->ReadBool( annexPhotoConfigured );
	savefile->ReadBool( annexPhonePulseBright );
	savefile->ReadInt( annexPhoneNextPulseTime );

	savefile->ReadBool( annexFiledPersonActive );
	savefile->ReadInt( annexFiledPersonNextMoveTime );
	savefile->ReadInt( annexFiledPersonObservedSince );

	BecomeActive( TH_THINK );
}
