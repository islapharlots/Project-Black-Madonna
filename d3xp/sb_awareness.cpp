#include "sys/platform.h"

#include "Game_local.h"
#include "sb_record.h"
#include "sb_awareness.h"

CLASS_DECLARATION( idEntity, idStBrielleAwarenessDirector )
END_CLASS

idStBrielleAwarenessDirector::idStBrielleAwarenessDirector() {
	lastTier = -1;
	nextCheckTime = 0;
	checkIntervalMS = 250;
	debug = false;
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

	nextCheckTime = gameLocal.time;
	BecomeActive( TH_THINK );
}

void idStBrielleAwarenessDirector::Think( void ) {
	if ( gameLocal.time < nextCheckTime ) {
		return;
	}

	nextCheckTime = gameLocal.time + checkIntervalMS;

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

void idStBrielleAwarenessDirector::Save( idSaveGame *savefile ) const {
	savefile->WriteInt( lastTier );
	savefile->WriteInt( nextCheckTime );
	savefile->WriteInt( checkIntervalMS );
	savefile->WriteBool( debug );
}

void idStBrielleAwarenessDirector::Restore( idRestoreGame *savefile ) {
	savefile->ReadInt( lastTier );
	savefile->ReadInt( nextCheckTime );
	savefile->ReadInt( checkIntervalMS );
	savefile->ReadBool( debug );

	BecomeActive( TH_THINK );
}
