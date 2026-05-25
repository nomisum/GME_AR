/*!
Fullscreen player map (M-key) overlay component.

Mirrors [[gme-gm-editor-map-overlay-ui-component]] but for the regular player
map screen. Only renders when the local player has the GM editor open — the
IsGmModeActive() guard inside the helper Tick is the sole gate, so non-GM
players never see anything.

Registered into the fullscreen map config — see [[gme-fullscreen-map-config]].
*/
class GME_GMFullscreenMapOverlayUIComponent : SCR_MapUIBaseComponent
{
	protected ref GME_GMMapOverlayBase m_pHelper;

	//------------------------------------------------------------------------------------------------
	override void OnMapOpen(MapConfiguration config)
	{
		super.OnMapOpen(config);

		m_pHelper = new GME_GMMapOverlayBase();
		m_pHelper.Init(m_RootWidget, m_MapEntity);

		if (m_MapEntity)
		{
			m_MapEntity.GetOnMapPan().Insert(OnPan);
			m_MapEntity.GetOnMapZoom().Insert(OnZoom);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnMapClose(MapConfiguration config)
	{
		if (m_MapEntity)
		{
			m_MapEntity.GetOnMapPan().Remove(OnPan);
			m_MapEntity.GetOnMapZoom().Remove(OnZoom);
		}

		if (m_pHelper)
		{
			m_pHelper.Shutdown();
			m_pHelper = null;
		}

		super.OnMapClose(config);
	}

	//------------------------------------------------------------------------------------------------
	override void Update(float timeSlice)
	{
		super.Update(timeSlice);

		if (m_pHelper)
			m_pHelper.Tick(timeSlice);
	}

	//------------------------------------------------------------------------------------------------
	protected bool OnPan(float x, float y, bool input)
	{
		if (m_pHelper)
			m_pHelper.OnPanOrZoom();
		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnZoom(float z)
	{
		if (m_pHelper)
			m_pHelper.OnPanOrZoom();
	}
}
