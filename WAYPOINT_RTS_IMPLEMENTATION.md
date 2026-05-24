# RTS-Style Waypoint Interaction - GME_AR Implementation

## Overview

This document describes the RTS-style waypoint interaction system implemented in the GME_AR addon for Arma Reforger Game Master editor. The system provides intuitive waypoint placement and type switching similar to real-time strategy games.

## Features

### 1. Immediate Waypoint Placement on Right-Click
- **Action**: Right-click on terrain with an AI group selected
- **Behavior**: Immediately places a waypoint at the cursor position without opening any menu
- **Modifier**: Hold Shift (EditorModifier) to queue waypoints instead of replacing
- **Context**: Only works when a group is selected; right-clicking on existing waypoints still opens the context menu

### 2. Waypoint Type Switching via Toolbar Buttons
- **Action**: Click a waypoint type button (Move, Defend, etc.) in the command toolbar
- **Behavior**: Changes the type of the group's current waypoint to the selected type at its existing position
- **Highlight**: The currently active waypoint type is highlighted with a background widget
- **Persistence**: The highlight persists across reselection of units via `GME_m_LastChosenAction`

### 3. Context Menu Suppression
- **Action**: Right-click on terrain with a group selected
- **Behavior**: Context menu (SCR_ContextActionsEditorComponent menu) does not appear
- **Method**: Flag-based suppression via `GME_m_bSuppressMenu` set during RMB terrain clicks

## Technical Implementation

### Files Modified/Created

#### 1. **SCR_CommandActionsEditorComponent.c** (Modified)
**Location**: `addons/GME/Scripts/Game/GME/Editor/Components/Editor/SCR_CommandActionsEditorComponent.c`

**Purpose**: Manages waypoint actions and RMB terrain placement logic

**Key Fields**:
- `bool GME_m_bSuppressMenu` — Flag to suppress context menu on terrain RMB
- `SCR_BaseEditorAction GME_m_LastChosenAction` — Stores the last selected waypoint type (persists across reselection)
- `vector GME_m_vLastWaypointPos` — Stores the last placed waypoint position (fallback for type changes)

**Key Methods**:
- `void GME_ReplaceWaypointWithAction(SCR_BaseEditorAction action)` — Finds the selected group's current waypoint and replaces it with a new type at the same position
  - Iterates `m_SelectedEntities` (protected field inherited from `SCR_BaseActionsEditorComponent`)
  - Handles both direct group selection and character selection (via `GetAIGroup()`)
  - Falls back to stored `GME_m_vLastWaypointPos` if selection state clears
  - Calls `ActionPerform` with evaluated flags from `ValidateSelection`

**How RMB Placement Works**:
1. `EOnEditorActivate` registers `MouseRight DOWN` listener
2. `OnRightMouseDown` fires when user right-clicks with mouse held down
3. Check `ValidateSelection` for `EEditorCommandActionFlags.WAYPOINT` (group selected)
4. Return early if hovering a waypoint entity (allow vanilla context menu)
5. Find the default action via `GetAndEvaluateActions` and `m_DefaultActionGroups`
6. Call `PerformDefaultAction(isQueue)` to place waypoint
7. Call `SetCurrentAction(actionToMark)` AFTER placement (vanilla resets it during placement)
8. Set `GME_m_bSuppressMenu = true` to suppress context menu on RMB UP
9. Store position in `GME_m_vLastWaypointPos` for type-change fallback

---

#### 2. **SCR_CommandToolbarEditorUIComponent.c** (Modified)
**Location**: `addons/GME/Scripts/Game/GME/Editor/UI/Components/SCR_CommandToolbarEditorUIComponent.c`

**Purpose**: Highlights active waypoint type and handles type-change clicks

**Key Modifications**:
- `OnClick` override — Calls `GME_ReplaceWaypointWithAction` instead of vanilla `PerformDefaultAction`
- `RefreshHighlights` override — Highlights active type using `GME_m_LastChosenAction` as fallback when `IsActionCurrent` returns false after reselection

**How It Works**:
1. Toolbar calls `Refresh()` on selection change
2. `Refresh()` subscribes to `GetOnCurrentActionChanged` invoker (first time only)
3. `Refresh()` calls `RefreshHighlights()`
4. `RefreshHighlights()` iterates toolbar items:
   - Gets current action via `IsActionCurrent` (live state)
   - Falls back to `GME_m_LastChosenAction` if null (after reselection)
   - Sets `Background` widget visibility for highlighted item
5. When user clicks toolbar button:
   - `OnClick` casts `m_EditorActionsComponent` to `SCR_CommandActionsEditorComponent`
   - Calls `GME_ReplaceWaypointWithAction(action)` to perform type change
   - Returns true to consume the click

---

#### 3. **SCR_ContextActionsEditorComponent.c** (Created)
**Location**: `addons/GME/Scripts/Game/GME/Editor/Components/Editor/SCR_ContextActionsEditorComponent.c`

**Purpose**: Suppresses context menu when RMB-clicking terrain with group selected

**Implementation**:
- Modded class inheriting from `SCR_BaseActionsEditorComponent`
- Overrides `EvaluateActions` (called when context menu is about to open)
- Checks `GME_m_bSuppressMenu` flag from `SCR_CommandActionsEditorComponent`
- If flag is true:
  - Clears the flag for next click
  - Returns early (returns empty `filteredActions`)
  - Context menu has no actions to display, so it doesn't open

---

#### 4. **SCR_ActionsRadialMenuEditorComponent.c** (Modified)
**Location**: `addons/GME/Scripts/Game/GME/Editor/Components/Editor/SCR_ActionsRadialMenuEditorComponent.c`

**Purpose**: Placeholder; minimal override to avoid conflicts

**Current State**: Empty modded class (all gamepad/radial menu logic removed)

---

#### 5. **SCR_BaseCommandAction.c** (Created)
**Location**: `addons/GME/Scripts/Game/GME/Editor/Actions/SCR_BaseCommandAction.c`

**Purpose**: Expose protected `m_CommandPrefab` field for future use

**Implementation**:
```enforce
modded class SCR_BaseCommandAction : SCR_BaseEditorAction
{
    ResourceName GME_GetCommandPrefab()
    {
        return m_CommandPrefab;
    }
}
```

(Not currently used, but available if needed for waypoint type introspection)

---

### Architecture Decisions

#### Why Not Override Vanilla Layouts?
Vanilla editor layouts are packed and not easily overridable. Instead, we:
- Hook into existing components via modded classes
- Leverage protected fields (`m_SelectedEntities`, `m_EditorActionsComponent`)
- Use flag-based communication between components (`GME_m_bSuppressMenu`)

#### Why Store `GME_m_LastChosenAction`?
After `ActionPerform`, vanilla's `OnPlacingSelectedPrefabChange` callback resets `m_CurrentAction` to null. The highlight would disappear if we only relied on `IsActionCurrent`. By storing the last user choice, we can restore the highlight after reselection.

#### Why Fallback to `GME_m_vLastWaypointPos`?
When a user clicks a toolbar button immediately after placement (single-click sequence), `m_SelectedEntities` may only contain the individual unit, not the group. By storing the waypoint position, we can still perform the type change even if the group isn't directly in the selection set.

#### Why Cast Selected Entity to `GetAIGroup()`?
Users typically click on individual units in a group, not the group entity itself. The selection contains `SCR_EditableCharacterComponent` entities. Calling `GetAIGroup()` on these returns the parent `SCR_EditableGroupComponent`, allowing type changes to work immediately without requiring reselection.

---

## Known Limitations & Future Improvements

### Current Limitations
1. **No Multi-Group Type Changes**: Clicking a type button only changes the first group's waypoint. Multi-group selection would require iterating all groups.
2. **No Waypoint Queue Visualization**: Visual feedback for queued waypoints (e.g., numbered overlays) is not implemented.
3. **Highlight Persistence**: `GME_m_LastChosenAction` persists even when switching to a group with no waypoint yet. Shows "stale" type until a new waypoint is placed.

### Potential Improvements
1. **Multi-Group Support**: Extend `GME_ReplaceWaypointWithAction` to iterate all groups in selection and change all their types
2. **Queue Indicators**: Add visual elements to show waypoint order when queuing
3. **Type Persistence per Group**: Store the "current type" per group instead of globally
4. **Keyboard Shortcuts**: Bind number keys (1=Move, 2=Defend, etc.) for quick type switching
5. **Waypoint Preview**: Show a preview of the new waypoint before confirming the type change

---

## Testing Checklist

- [x] RMB on terrain with group selected → waypoint placed immediately
- [x] RMB with Shift held → waypoint queued (replaces vs. appends based on mode)
- [x] RMB on waypoint entity → context menu opens (vanilla behavior)
- [x] Click toolbar type button → waypoint type changed at existing position
- [x] Toolbar button highlights current type
- [x] Highlight persists after reselecting units
- [x] Selecting individual unit in group → type change still works (via `GetAIGroup()`)
- [x] Context menu suppressed on terrain RMB (flag-based)

---

## Debugging Notes

### Debug Prints
`SCR_CommandActionsEditorComponent.GME_ReplaceWaypointWithAction` includes detailed logging:
- `[GME] ReplaceWaypoint: selectedEntities=X` — Count of selected entities
- `[GME] ReplaceWaypoint: group found, children=X` — Waypoint children found
- `[GME] ReplaceWaypoint: waypoint found, IsCurrent=1/0` — Waypoint state
- `[GME] ReplaceWaypoint: pos=1 (stored=<x,y,z>)` — Position stored successfully

### Common Issues & Solutions

**Issue**: Toolbar button click doesn't change waypoint type
- **Cause**: `m_SelectedEntities` empty or doesn't contain group
- **Solution**: Verify `GetAIGroup()` cast is working; check if individual character selected
- **Debug**: Add print in `GME_ReplaceWaypointWithAction` to see entity count and types

**Issue**: Highlight disappears after placing first waypoint
- **Cause**: `m_CurrentAction` reset by vanilla; `RefreshHighlights` not called
- **Solution**: Verify `GetOnCurrentActionChanged` subscriber is attached; check `Refresh()` fires
- **Debug**: Add print in `RefreshHighlights` to confirm it's being called

**Issue**: Context menu still appears on terrain RMB
- **Cause**: `GME_m_bSuppressMenu` flag not set or cleared too early
- **Solution**: Verify `OnRightMouseDown` fires before context menu evaluation
- **Debug**: Add print in `SCR_ContextActionsEditorComponent.EvaluateActions` to see flag state

---

## Related Vanilla Classes

- `SCR_BaseActionsEditorComponent` — Base for all action managers; provides `m_SelectedEntities`, `ValidateSelection`, `ActionPerform`
- `SCR_CommandActionsEditorComponent` — Vanilla waypoint action manager; parent of our modded class
- `SCR_EditableGroupComponent` — Editable AI group; provides `GetChildrenRef()`, `GetWaypointCount()`
- `SCR_EditableWaypointComponent` — Editable waypoint entity; provides `IsCurrent()`, `GetAIGroup()`
- `SCR_ContextActionsEditorComponent` — Vanilla context menu manager; our hook point for suppression
- `SCR_BaseCommandAction` — Waypoint type action base; holds `m_CommandPrefab` (protected)

---

## Files Summary

| File | Type | Purpose |
|------|------|---------|
| `SCR_CommandActionsEditorComponent.c` | Modded | RMB placement & type change logic |
| `SCR_CommandToolbarEditorUIComponent.c` | Modded | Toolbar highlighting & click handling |
| `SCR_ContextActionsEditorComponent.c` | Modded | Context menu suppression |
| `SCR_ActionsRadialMenuEditorComponent.c` | Modded | Gamepad radial menu (minimal) |
| `SCR_BaseCommandAction.c` | Modded | Expose waypoint prefab (unused) |

**Total Lines of Code**: ~200 (excluding debug prints and comments)
