/*!
GM editor (sky-view) map overlay component.

Renders the live unit & vehicle marker overlay on the GM editor map. All real
work lives in [[gme-gm-map-overlay-base]]; this class wires lifecycle hooks and
the editor-config gate.

Registered into the editor map config — see [[gme-editor-map-config]].
*/
class GME_GMEditorMapOverlayUIComponent : SCR_MapEditorUIComponent
{
	protected ref GME_GMMapOverlayBase m_pHelper;

	//------------------------------------------------------------------------------------------------
	override void OnMapOpen(MapConfiguration config)
	{
		super.OnMapOpen(config);

		if (!IsConfigEditor(config))
			return;

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
	override void OnMenuUpdate(float timeSlice)
	{
		super.OnMenuUpdate(timeSlice);

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
