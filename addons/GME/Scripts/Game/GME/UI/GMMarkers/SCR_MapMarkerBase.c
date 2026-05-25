/*!
GM marker extension — adds a "global" flag (visible to all factions) on top of
vanilla static markers.

STUB — vanilla `SCR_MapMarkerBase` is hand-rolled (non-entity) replication.
This file declares the new flag and its accessors but the network serialization
hooks must be wired into vanilla's read/write pair before merge.

TODOs (must be resolved before this is functional):
1. Open vanilla SCR_MapMarkerBase.c in the base editor addon (58D0FB3206B6F859);
   locate its `Write(ScriptBitWriter)` / `Read(ScriptBitReader)` (or equivalently
   named) pair and confirm the override signature.
2. Append a single bit for `m_bGME_IsGlobal` to both Write and Read overrides
   here. Mind backwards-compat — old replicated streams will not include this
   bit, so guard accordingly (default false on read failure).
3. If serialization can't be cleanly extended, switch to the sentinel-ownerID
   fallback documented in the plan: reserve a magic ownerID (e.g. -2) for
   "global" markers and have `GME_IsGlobal()` interpret that, dropping the
   member entirely.
*/
modded class SCR_MapMarkerBase
{
	protected bool m_bGME_IsGlobal;

	//------------------------------------------------------------------------------------------------
	bool GME_IsGlobal()
	{
		return m_bGME_IsGlobal;
	}

	//------------------------------------------------------------------------------------------------
	void GME_SetGlobal(bool global)
	{
		m_bGME_IsGlobal = global;
	}

	// TODO override Write/Read once vanilla signatures are known:
	//
	// override bool Write(ScriptBitWriter writer)
	// {
	//     if (!super.Write(writer)) return false;
	//     writer.WriteBool(m_bGME_IsGlobal);
	//     return true;
	// }
	//
	// override bool Read(ScriptBitReader reader)
	// {
	//     if (!super.Read(reader)) return false;
	//     return reader.ReadBool(m_bGME_IsGlobal);
	// }
};
