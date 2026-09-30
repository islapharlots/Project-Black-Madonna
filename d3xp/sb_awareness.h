#pragma once

#include "Entity.h"

// Per-map interpreter for the global St. Brielle Awareness value.
//
// The director never exposes Awareness to the player directly. Instead it
// activates mapper-authored reactions when the persistent value enters a new
// tier. A map can have one director with any number of:
//
//   tier0_target*
//   tier1_target*
//   tier2_target*
//   tier3_target*
//   tier4_target*
//
// Each value is the name of an entity to activate when that tier becomes
// current.
//
// Spawn args:
//   check_interval_ms    Poll interval. Default 250.
//   fire_current_on_spawn  Fire the current tier once after map spawn.
//   awareness_debug      Print tier transitions to the console.
class idStBrielleAwarenessDirector : public idEntity {
public:
	CLASS_PROTOTYPE( idStBrielleAwarenessDirector );

	idStBrielleAwarenessDirector();

	virtual void		Spawn( void );
	virtual void		Think( void );
	virtual void		Save( idSaveGame *savefile ) const;
	virtual void		Restore( idRestoreGame *savefile );

private:
	int					lastTier;
	int					nextCheckTime;
	int					checkIntervalMS;
	bool				debug;

	void				ActivateTierTargets( int tier );
};
