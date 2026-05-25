/*!
Shared helper for the GM live unit/vehicle map overlay.

Drives the widget pool, world-to-screen projection, state classification, and
the time-accumulator update gate. Owned by GME_GMEditorMapOverlayUIComponent
(editor sky-view map) and GME_GMFullscreenMapOverlayUIComponent (regular M-key
map). Both components delegate identical work here so the only per-screen
difference lives in the lifecycle hooks.
*/
class GME_GMMapOverlayBase : Managed
{
	static const ResourceName MARKER_LAYOUT = "{00000000DEADBEEF}UI/layouts/GMMarkers/GME_GMUnitMarker.layout"; // TODO replace GUID once layout asset is registered
	static const float UPDATE_INTERVAL = 0.1; // 10 Hz
	static const int DEFAULT_POOL_SIZE = 200;
	static const int ICON_W = 30;
	static const int ICON_H = 32;

	protected ref array<Widget> m_aWidgetPool = {};
	protected Widget m_pRootWidget;
	protected SCR_MapEntity m_pMapEntity;
	protected float m_fAccumulator;
	protected bool m_bForceRedraw;
	protected bool m_bWarnedPoolOverflow;

	//------------------------------------------------------------------------------------------------
	void Init(Widget root, SCR_MapEntity mapEntity, int poolSize = DEFAULT_POOL_SIZE)
	{
		m_pRootWidget = root;
		m_pMapEntity = mapEntity;
		m_bForceRedraw = true;
		m_fAccumulator = 0;
		m_bWarnedPoolOverflow = false;

		WorkspaceWidget ws = GetGame().GetWorkspace();
		if (!ws || !root)
			return;

		for (int i = 0; i < poolSize; i++)
		{
			Widget w = ws.CreateWidgets(MARKER_LAYOUT, root);
			if (!w)
				break;

			w.SetVisible(false);
			m_aWidgetPool.Insert(w);
		}
	}

	//------------------------------------------------------------------------------------------------
	void Shutdown()
	{
		foreach (Widget w : m_aWidgetPool)
		{
			if (w)
				w.RemoveFromHierarchy();
		}
		m_aWidgetPool.Clear();
		m_pRootWidget = null;
		m_pMapEntity = null;
	}

	//------------------------------------------------------------------------------------------------
	void OnPanOrZoom()
	{
		m_bForceRedraw = true;
	}

	//------------------------------------------------------------------------------------------------
	//! Per-frame entry. Returns early if the local player isn't in GM editor mode.
	void Tick(float timeSlice)
	{
		if (!IsGmModeActive())
		{
			HideAll();
			return;
		}

		m_fAccumulator += timeSlice;
		if (!m_bForceRedraw && m_fAccumulator < UPDATE_INTERVAL)
			return;

		m_fAccumulator = 0;
		m_bForceRedraw = false;
		Redraw();
	}

	//------------------------------------------------------------------------------------------------
	//! True only when the local player is in the editor sky-view (not just having GM permissions).
	static bool IsGmModeActive()
	{
		SCR_EditorManagerEntity mgr = SCR_EditorManagerEntity.GetInstance();
		return mgr && mgr.IsOpened();
	}

	//------------------------------------------------------------------------------------------------
	protected void HideAll()
	{
		foreach (Widget w : m_aWidgetPool)
		{
			if (w && w.IsVisible())
				w.SetVisible(false);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Redraw()
	{
		if (!m_pMapEntity || m_aWidgetPool.IsEmpty())
			return;

		WorkspaceWidget ws = GetGame().GetWorkspace();
		if (!ws)
			return;

		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (!core)
			return;

		set<SCR_EditableEntityComponent> entities = new set<SCR_EditableEntityComponent>();
		core.GetAllEntities(entities);

		int idx = 0;
		int poolSize = m_aWidgetPool.Count();

		foreach (SCR_EditableEntityComponent ent : entities)
		{
			if (!ent)
				continue;

			IEntity owner = ent.GetOwner();
			if (!owner)
				continue;

			// AI/non-player units enumerate identically to player units; do NOT filter by player control.
			bool isCharacter = SCR_EditableCharacterComponent.Cast(ent) != null;
			bool isVehicle = !isCharacter && IsVehicleEntity(ent);
			if (!isCharacter && !isVehicle)
				continue;

			if (idx >= poolSize)
			{
				if (!m_bWarnedPoolOverflow)
				{
					Print("[GME] Map overlay pool exhausted; some entities will not render this frame.", LogLevel.WARNING);
					m_bWarnedPoolOverflow = true;
				}
				break;
			}

			vector pos = owner.GetOrigin();
			float sx, sy;
			m_pMapEntity.WorldToScreen(pos[0], pos[2], sx, sy, true);
			sx = ws.DPIUnscale(sx);
			sy = ws.DPIUnscale(sy);

			Widget w = m_aWidgetPool[idx];
			FrameSlot.SetPos(w, sx - (ICON_W / 2), sy - (ICON_H / 2));

			Color c;
			if (isCharacter)
				c = ClassifyCharacterState(owner);
			else
				c = ClassifyVehicleState(owner);

			ApplyTint(w, c);
			w.SetVisible(true);
			idx++;
		}

		for (int i = idx; i < poolSize; i++)
		{
			Widget w = m_aWidgetPool[i];
			if (w && w.IsVisible())
				w.SetVisible(false);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Heuristic vehicle filter — Reforger has no `SCR_EditableVehicleComponent` cast in this addon's
	//! existing usage, so we look for a compartment manager on the owner. Refine if the editable
	//! entity type system exposes a cleaner predicate.
	protected static bool IsVehicleEntity(SCR_EditableEntityComponent ent)
	{
		IEntity owner = ent.GetOwner();
		if (!owner)
			return false;

		BaseCompartmentManagerComponent compMgr = BaseCompartmentManagerComponent.Cast(owner.FindComponent(BaseCompartmentManagerComponent));
		return compMgr != null;
	}

	//------------------------------------------------------------------------------------------------
	protected static void ApplyTint(Widget root, Color color)
	{
		if (!root)
			return;

		ImageWidget icon = ImageWidget.Cast(root.FindAnyWidget("Icon"));
		if (icon)
			icon.SetColor(color);
	}

	//------------------------------------------------------------------------------------------------
	//! Dead/destroyed and unconscious are surfaced with distinct visuals; ALIVE tints by faction.
	protected static Color ClassifyCharacterState(IEntity character)
	{
		ChimeraCharacter chim = ChimeraCharacter.Cast(character);
		if (!chim)
			return Color.FromRGBA(255, 255, 255, 255);

		SCR_CharacterDamageManagerComponent dmg = SCR_CharacterDamageManagerComponent.Cast(chim.GetDamageManager());
		if (dmg && dmg.GetState() == EDamageState.DESTROYED)
			return Color.FromRGBA(220, 50, 50, 128); // DEAD: red, 0.5 alpha

		CharacterControllerComponent ctrl = chim.GetCharacterController();
		if (ctrl && ctrl.IsUnconscious())
			return Color.FromRGBA(230, 200, 60, 217); // UNCON: yellow, 0.85 alpha

		// ALIVE — faction tint, fallback green.
		FactionAffiliationComponent aff = FactionAffiliationComponent.Cast(chim.FindComponent(FactionAffiliationComponent));
		if (aff)
		{
			Faction f = aff.GetAffiliatedFaction();
			if (f)
				return f.GetFactionColor();
		}

		return Color.FromRGBA(80, 220, 100, 255);
	}

	//------------------------------------------------------------------------------------------------
	protected static Color ClassifyVehicleState(IEntity vehicle)
	{
		SCR_DamageManagerComponent dmg = SCR_DamageManagerComponent.Cast(vehicle.FindComponent(SCR_DamageManagerComponent));
		if (dmg && dmg.GetState() == EDamageState.DESTROYED)
			return Color.FromRGBA(150, 30, 30, 128); // DESTROYED: dark red, 0.5 alpha

		BaseCompartmentManagerComponent compMgr = BaseCompartmentManagerComponent.Cast(vehicle.FindComponent(BaseCompartmentManagerComponent));
		if (compMgr)
		{
			array<BaseCompartmentSlot> slots = {};
			compMgr.GetCompartments(slots);
			foreach (BaseCompartmentSlot slot : slots)
			{
				if (slot && slot.GetOccupant())
					return Color.FromRGBA(255, 255, 255, 255); // OCCUPIED
			}
		}

		return Color.FromRGBA(160, 160, 160, 217); // EMPTY/UNMANNED: grey, 0.85 alpha
	}
}
