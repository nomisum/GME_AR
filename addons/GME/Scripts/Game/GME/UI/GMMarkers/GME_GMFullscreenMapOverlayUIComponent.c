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
			m_MapEntity.GetOnMapPan().Insert(OnPanZoom);
			m_MapEntity.GetOnMapZoom().Insert(OnPanZoom);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnMapClose(MapConfiguration config)
	{
		if (m_MapEntity)
		{
			m_MapEntity.GetOnMapPan().Remove(OnPanZoom);
			m_MapEntity.GetOnMapZoom().Remove(OnPanZoom);
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
	protected void OnPanZoom(float x, float y)
	{
		if (m_pHelper)
			m_pHelper.OnPanOrZoom();
	}
}
