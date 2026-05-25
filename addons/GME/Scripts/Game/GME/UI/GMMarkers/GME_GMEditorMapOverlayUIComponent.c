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
		m_pHelper.Init(GetWidget(), m_MapEntity);

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
	override void OnMenuUpdate()
	{
		super.OnMenuUpdate();

		if (m_pHelper)
			m_pHelper.Tick(GME_GMMapOverlayBase.UPDATE_INTERVAL);
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
