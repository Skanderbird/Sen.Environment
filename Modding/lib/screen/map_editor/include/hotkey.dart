import 'dart:collection';

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_bloc/flutter_bloc.dart';
import 'package:sen/cubit/map_editor_configuration_cubit/map_editor_configuration_cubit.dart';
import 'package:sen/model/worldmap.dart';
import 'package:sen/screen/map_editor/bloc/history/history_bloc.dart';
import 'package:sen/screen/map_editor/bloc/item/item_bloc.dart';
import 'package:sen/screen/map_editor/bloc/section/section_bloc.dart';
import 'package:sen/screen/map_editor/bloc/selected/selected_bloc.dart';
import 'package:sen/screen/map_editor/bloc/setting/setting_bloc.dart';
import 'package:sen/screen/map_editor/bloc/stage/stage_bloc.dart';
import 'package:sen/screen/map_editor/bloc/toolbar/toolbar_bloc.dart';
import 'package:sen/screen/map_editor/include/controller.dart';
import 'package:sen/screen/map_editor/include/hotkey_listener.dart';
import 'package:sen/screen/map_editor/bloc/layer/layer_bloc.dart';
import 'package:sen/screen/map_editor/models/action_model.dart';
import 'package:sen/screen/map_editor/models/action_service.dart';
import 'package:sen/screen/map_editor/models/map_const.dart';

class Hotkey extends StatelessWidget {
  const Hotkey({super.key, required this.controller, required this.child});

  final Widget child;

  final CanvasController controller;

  void _delete(BuildContext context) {
    final stageBloc = context.read<StageBloc>();
    final layerBloc = context.read<LayerBloc>();
    final selectedList = context.read<SelectedBloc>().state.selectedList;
    context.read<StageBloc>().add(
          RemoveItemEvent(
            idList: selectedList,
            itemBloc: context.read<ItemBloc>(),
            layerBloc: context.read<LayerBloc>(),
          ),
        );
    final actionService = ActionService<ActionModelType>(
      actionType: ActionType.eraseItem,
      data: {
        ActionModelType.mapPieces: stageBloc.clonePieces(),
        ActionModelType.mapEvents: stageBloc.cloneEvents(),
      },
      change: (data) {
        final pieces =
            data![ActionModelType.mapPieces] as HashMap<String, MapPieceItem>;
        final events =
            data[ActionModelType.mapEvents] as HashMap<String, MapEventItem>;
        stageBloc.add(UpdateStageState(
            stageState: stageBloc.state.copyWith(
          events: events,
          pieces: pieces,
        )));
        final layerKeys = <int>[];
        for (final e in pieces.values) {
          if (!layerKeys.contains(e.layer)) {
            layerKeys.add(e.layer);
          }
        }
        for (final layerId in layerKeys) {
          layerBloc.updateNodeParallax(layerId, pieceItems: pieces);
        }
        layerBloc.updateTree(true);
      },
    );
    context.read<HistoryBloc>().add(CaptureState(state: actionService));
  }

  void _copy(BuildContext context) {
    controller.catchPointerHover();
    context.read<SelectedBloc>().add(const CopySelectedList(isCut: false));
  }

  void _cut(BuildContext context) {
    controller.catchPointerHover();
    context.read<SelectedBloc>().add(const CopySelectedList(isCut: true));
  }

  @override
  Widget build(BuildContext context) {
    final toolbarBloc = context.read<ToolBarBloc>();
    final stageBloc = context.read<StageBloc>();
    final layerBloc = context.read<LayerBloc>();
    return HotkeyListener(
        controller: controller,
        onKeyDownEvent: (logicalKey) {
          if (controller.controlPressed || controller.metaPressed) {
            switch (logicalKey) {
              case LogicalKeyboardKey.keyX:
                {
                  _cut(context);
                  break;
                }
              case LogicalKeyboardKey.keyC:
                {
                  _copy(context);
                  break;
                }
              case LogicalKeyboardKey.keyV:
                {
                  final copyList = context.read<SelectedBloc>().state.copyList;
                  final details = controller.getDetailsMove();
                  if (copyList.isNotEmpty) {
                    if (copyList.contains('paste_cut')) {
                      for (final id in copyList) {
                        final itemProfile =
                            context.read<ItemBloc>().state.itemStore[id];
                        if (itemProfile != null) {
                          if (itemProfile.isEvent) {
                            final event = stageBloc.state.events[id]!;
                            event.position.x += details.dx;
                            event.position.y += details.dy;
                          } else {
                            final piece = stageBloc.state.pieces[id]!;
                            piece.position.x += details.dx;
                            piece.position.y += details.dy;
                          }
                        }
                      }
                      if (!context.read<SettingBloc>().state.muteAudio) {
                        context
                            .read<MapEditorConfigurationCubit>()
                            .state
                            .editorResource
                            .setItemSound
                            .resume();
                      }
                      context.read<SelectedBloc>().add(const ClearCopyList());
                      context.read<ItemBloc>().add(const ItemStoreUpdated());
                    } else {
                      context.read<StageBloc>().add(CloneItemEvent(
                            idList: copyList,
                            itemBloc: context.read<ItemBloc>(),
                            layerBloc: context.read<LayerBloc>(),
                            positionXAdditional: details.dx,
                            positionYAdditional: details.dy,
                          ));
                    }
                    final actionService = ActionService<ActionModelType>(
                        actionType: ActionType.pasteItem,
                        data: {
                          ActionModelType.mapPieces: stageBloc.clonePieces(),
                          ActionModelType.mapEvents: stageBloc.cloneEvents(),
                        },
                        change: (data) {
                          final pieces = data![ActionModelType.mapPieces]
                              as HashMap<String, MapPieceItem>;
                          final events = data[ActionModelType.mapEvents]
                              as HashMap<String, MapEventItem>;
                          stageBloc.add(UpdateStageState(
                              stageState: stageBloc.state
                                  .copyWith(events: events, pieces: pieces)));
                          final layerKeys = <int>[];
                          for (final e in pieces.values) {
                            if (!layerKeys.contains(e.layer)) {
                              layerKeys.add(e.layer);
                            }
                          }
                          for (final layerId in layerKeys) {
                            layerBloc.updateNodeParallax(layerId,
                                pieceItems: pieces);
                          }
                          layerBloc.updateTree(true);
                          // historyBloc.add(const UpdateIndexEvent());
                        });
                    context
                        .read<HistoryBloc>()
                        .add(CaptureState(state: actionService));
                  }
                  break;
                }
              default:
                {
                  toolbarBloc.add(const ToolToogled(
                      type: ToolType.rectangleTool, enabled: true));
                  break;
                }
            }
          } else {
            switch (logicalKey) {
              case LogicalKeyboardKey.keyE:
                {
                  toolbarBloc.add(const ToolToogled(
                      type: ToolType.eraseTool, enabled: true));
                  break;
                }
              case LogicalKeyboardKey.keyW:
                {
                  final viewportScale = controller.matrix.getMaxScaleOnAxis();
                  context.read<StageBloc>().add(UpdateItemPosition(
                        x: 0,
                        y: -1 / viewportScale,
                        itemBloc: context.read<ItemBloc>(),
                      ));
                  context.read<ItemBloc>().add(const ItemStoreUpdated());
                  break;
                }
              case LogicalKeyboardKey.keyA:
                {
                  final viewportScale = controller.matrix.getMaxScaleOnAxis();
                  context.read<StageBloc>().add(UpdateItemPosition(
                        x: -1 / viewportScale,
                        y: 0,
                        itemBloc: context.read<ItemBloc>(),
                      ));
                  context.read<ItemBloc>().add(const ItemStoreUpdated());
                  break;
                }
              case LogicalKeyboardKey.keyS:
                {
                  final viewportScale = controller.matrix.getMaxScaleOnAxis();
                  context.read<StageBloc>().add(UpdateItemPosition(
                        x: 0,
                        y: 1 / viewportScale,
                        itemBloc: context.read<ItemBloc>(),
                      ));
                  context.read<ItemBloc>().add(const ItemStoreUpdated());
                  break;
                }
              case LogicalKeyboardKey.keyD:
                {
                  final viewportScale = controller.matrix.getMaxScaleOnAxis();
                  context.read<StageBloc>().add(UpdateItemPosition(
                        x: 1 / viewportScale,
                        y: 0,
                        itemBloc: context.read<ItemBloc>(),
                      ));
                  context.read<ItemBloc>().add(const ItemStoreUpdated());
                  break;
                }
              case LogicalKeyboardKey.keyZ:
                {
                  final selectedList =
                      context.read<SelectedBloc>().state.selectedList;
                  for (final id in selectedList) {
                    final itemProfile =
                        context.read<ItemBloc>().state.itemStore[id];
                    if (itemProfile != null) {
                      if (!itemProfile.isEvent) {
                        final piece = stageBloc.state.pieces[id]!;
                        if (piece.pieceType == PieceType.animation) {
                          final angle =
                              (piece.rotationAngle * MapConst.pi / 180) - 0.1;
                          piece.rotationAngle =
                              (angle / MapConst.pi * 180).round();
                        } else {
                          piece.rotationAngle += 1;
                        }
                      }
                    }
                  }
                  context.read<ItemBloc>().add(const ItemStoreUpdated());
                  break;
                }
              case LogicalKeyboardKey.keyX:
                {
                  final selectedList =
                      context.read<SelectedBloc>().state.selectedList;
                  for (final id in selectedList) {
                    final itemProfile =
                        context.read<ItemBloc>().state.itemStore[id];
                    if (itemProfile != null) {
                      if (!itemProfile.isEvent) {
                        final piece = stageBloc.state.pieces[id]!;
                        if (piece.pieceType == PieceType.animation) {
                          final angle =
                              (piece.rotationAngle * MapConst.pi / 180) + 0.1;
                          piece.rotationAngle =
                              (angle / MapConst.pi * 180).round();
                        } else {
                          piece.rotationAngle -= 1;
                        }
                      }
                    }
                  }
                  context.read<ItemBloc>().add(const ItemStoreUpdated());
                  break;
                }
              case LogicalKeyboardKey.keyJ:
                {
                  final selectedList =
                      context.read<SelectedBloc>().state.selectedList;
                  for (final id in selectedList) {
                    final itemProfile =
                        context.read<ItemBloc>().state.itemStore[id];
                    if (itemProfile != null) {
                      if (!itemProfile.isEvent) {
                        final piece = stageBloc.state.pieces[id]!;
                        piece.scaleX -= 0.1;
                      }
                    }
                  }
                  context.read<ItemBloc>().add(const ItemStoreUpdated());
                  break;
                }
              case LogicalKeyboardKey.keyK:
                {
                  final selectedList =
                      context.read<SelectedBloc>().state.selectedList;
                  for (final id in selectedList) {
                    final itemProfile =
                        context.read<ItemBloc>().state.itemStore[id];
                    if (itemProfile != null) {
                      if (!itemProfile.isEvent) {
                        final piece = stageBloc.state.pieces[id]!;
                        piece.scaleX += 0.1;
                      }
                    }
                  }
                  context.read<ItemBloc>().add(const ItemStoreUpdated());
                  break;
                }
              case LogicalKeyboardKey.keyN:
                {
                  final selectedList =
                      context.read<SelectedBloc>().state.selectedList;
                  for (final id in selectedList) {
                    final itemProfile =
                        context.read<ItemBloc>().state.itemStore[id];
                    if (itemProfile != null) {
                      if (!itemProfile.isEvent) {
                        final piece = stageBloc.state.pieces[id]!;
                        piece.scaleY -= 0.1;
                      }
                    }
                  }
                  context.read<ItemBloc>().add(const ItemStoreUpdated());
                  break;
                }
              case LogicalKeyboardKey.keyM:
                {
                  final selectedList =
                      context.read<SelectedBloc>().state.selectedList;
                  for (final id in selectedList) {
                    final itemProfile =
                        context.read<ItemBloc>().state.itemStore[id];
                    if (itemProfile != null) {
                      if (!itemProfile.isEvent) {
                        final piece = stageBloc.state.pieces[id]!;
                        piece.scaleY += 0.1;
                      }
                    }
                  }
                  context.read<ItemBloc>().add(const ItemStoreUpdated());
                  break;
                }
              case LogicalKeyboardKey.space:
                {
                  toolbarBloc.add(
                      const ToolToogled(type: ToolType.panTool, enabled: true));
                  break;
                }
              default:
                break;
            }
          }
        },
        onKeyRepeatEvent: (logicalKey) {
          switch (logicalKey) {
            case LogicalKeyboardKey.keyW:
              {
                final viewportScale = controller.matrix.getMaxScaleOnAxis();
                context.read<StageBloc>().add(UpdateItemPosition(
                      x: 0,
                      y: -2 / viewportScale,
                      itemBloc: context.read<ItemBloc>(),
                    ));
                context.read<ItemBloc>().add(const ItemStoreUpdated());
                break;
              }
            case LogicalKeyboardKey.keyA:
              {
                final viewportScale = controller.matrix.getMaxScaleOnAxis();
                context.read<StageBloc>().add(UpdateItemPosition(
                      x: -2 / viewportScale,
                      y: 0,
                      itemBloc: context.read<ItemBloc>(),
                    ));
                context.read<ItemBloc>().add(const ItemStoreUpdated());
                break;
              }
            case LogicalKeyboardKey.keyS:
              {
                final viewportScale = controller.matrix.getMaxScaleOnAxis();
                context.read<StageBloc>().add(UpdateItemPosition(
                      x: 0,
                      y: 2 / viewportScale,
                      itemBloc: context.read<ItemBloc>(),
                    ));
                context.read<ItemBloc>().add(const ItemStoreUpdated());
                break;
              }
            case LogicalKeyboardKey.keyD:
              {
                final viewportScale = controller.matrix.getMaxScaleOnAxis();
                context.read<StageBloc>().add(UpdateItemPosition(
                      x: 2 / viewportScale,
                      y: 0,
                      itemBloc: context.read<ItemBloc>(),
                    ));
                context.read<ItemBloc>().add(const ItemStoreUpdated());
                break;
              }
            case LogicalKeyboardKey.keyJ:
              {
                final selectedList =
                    context.read<SelectedBloc>().state.selectedList;
                for (final id in selectedList) {
                  final itemProfile =
                      context.read<ItemBloc>().state.itemStore[id];
                  if (itemProfile != null) {
                    if (!itemProfile.isEvent) {
                      final piece = stageBloc.state.pieces[id]!;
                      final newScale = piece.scaleX - 0.1;
                      if (newScale >= 0.1 && newScale <= 10) {
                        piece.scaleX = newScale;
                      }
                    }
                  }
                }
                context.read<ItemBloc>().add(const ItemStoreUpdated());
                break;
              }
            case LogicalKeyboardKey.keyK:
              {
                final selectedList =
                    context.read<SelectedBloc>().state.selectedList;
                for (final id in selectedList) {
                  final itemProfile =
                      context.read<ItemBloc>().state.itemStore[id];
                  if (itemProfile != null) {
                    if (!itemProfile.isEvent) {
                      final piece = stageBloc.state.pieces[id]!;
                      final newScale = piece.scaleX + 0.1;
                      if (newScale >= 0.1 && newScale <= 10) {
                        piece.scaleX = newScale;
                      }
                    }
                  }
                }
                context.read<ItemBloc>().add(const ItemStoreUpdated());
                break;
              }
            case LogicalKeyboardKey.keyN:
              {
                final selectedList =
                    context.read<SelectedBloc>().state.selectedList;
                for (final id in selectedList) {
                  final itemProfile =
                      context.read<ItemBloc>().state.itemStore[id];
                  if (itemProfile != null) {
                    if (!itemProfile.isEvent) {
                      final piece = stageBloc.state.pieces[id]!;
                      final newScale = piece.scaleY - 0.1;
                      if (newScale >= 0.1 && newScale <= 10) {
                        piece.scaleY = newScale;
                      }
                    }
                  }
                }
                context.read<ItemBloc>().add(const ItemStoreUpdated());
                break;
              }
            case LogicalKeyboardKey.keyM:
              {
                final selectedList =
                    context.read<SelectedBloc>().state.selectedList;
                for (final id in selectedList) {
                  final itemProfile =
                      context.read<ItemBloc>().state.itemStore[id];
                  if (itemProfile != null) {
                    if (!itemProfile.isEvent) {
                      final piece = stageBloc.state.pieces[id]!;
                      final newScale = piece.scaleY + 0.1;
                      if (newScale >= 0.1 && newScale <= 10) {
                        piece.scaleY = newScale;
                      }
                    }
                  }
                }
                context.read<ItemBloc>().add(const ItemStoreUpdated());
                break;
              }
            case LogicalKeyboardKey.keyZ:
              {
                final selectedList =
                    context.read<SelectedBloc>().state.selectedList;
                for (final id in selectedList) {
                  final itemProfile =
                      context.read<ItemBloc>().state.itemStore[id];
                  if (itemProfile != null) {
                    if (!itemProfile.isEvent) {
                      final piece = stageBloc.state.pieces[id]!;
                      if (piece.pieceType == PieceType.animation) {
                        final angle =
                            (piece.rotationAngle * MapConst.pi / 180) - 0.1;
                        piece.rotationAngle =
                            (angle / MapConst.pi * 180).round();
                      } else {
                        piece.rotationAngle += 1;
                      }
                    }
                  }
                }
                context.read<ItemBloc>().add(const ItemStoreUpdated());
                break;
              }
            case LogicalKeyboardKey.keyX:
              {
                final selectedList =
                    context.read<SelectedBloc>().state.selectedList;
                for (final id in selectedList) {
                  final itemProfile =
                      context.read<ItemBloc>().state.itemStore[id];
                  if (itemProfile != null) {
                    if (!itemProfile.isEvent) {
                      final piece = stageBloc.state.pieces[id]!;
                      if (piece.pieceType == PieceType.animation) {
                        final angle =
                            (piece.rotationAngle * MapConst.pi / 180) + 0.1;
                        piece.rotationAngle =
                            (angle / MapConst.pi * 180).round();
                      } else {
                        piece.rotationAngle -= 1;
                      }
                    }
                  }
                }
                context.read<ItemBloc>().add(const ItemStoreUpdated());
                break;
              }
            default:
              break;
          }
        },
        onKeyUpEvent: (logicalKey) {
          if (controller.controlPressed || controller.metaPressed) {
            final sectionBloc = context.read<SectionBloc>();
            switch (logicalKey) {
              case LogicalKeyboardKey.keyO:
                {
                  toolbarBloc.add(ToolOpenEvent(
                    stageBloc: context.read<StageBloc>(),
                    itemBloc: context.read<ItemBloc>(),
                    layerBloc: context.read<LayerBloc>(),
                  ));
                }
              case LogicalKeyboardKey.keyS:
                {
                  toolbarBloc.add(ToolSaveEvent(
                    stageBloc: context.read<StageBloc>(),
                    // autosaveBloc: context.read<AutosaveBloc>(),
                  ));
                  break;
                }
              case LogicalKeyboardKey.keyR:
                {
                  toolbarBloc.add(const ToolToogled(type: ToolType.resizeTool));
                  break;
                }
              case LogicalKeyboardKey.digit1:
                {
                  const type = SectionType.select;
                  final isMinize = sectionBloc.state.sectionMinize[type]!;
                  isMinize && type == sectionBloc.state.section
                      ? sectionBloc.add(
                          const SectionMinizeToggled(type: type, minize: false))
                      : sectionBloc.add(const SetSection(section: type));
                  break;
                }
              case LogicalKeyboardKey.digit2:
                {
                  const type = SectionType.image;
                  final isMinize = sectionBloc.state.sectionMinize[type]!;
                  isMinize && type == sectionBloc.state.section
                      ? sectionBloc.add(
                          const SectionMinizeToggled(type: type, minize: false))
                      : sectionBloc.add(const SetSection(section: type));
                  break;
                }
              case LogicalKeyboardKey.digit3:
                {
                  const type = SectionType.animation;
                  final isMinize = sectionBloc.state.sectionMinize[type]!;
                  isMinize && type == sectionBloc.state.section
                      ? sectionBloc.add(
                          const SectionMinizeToggled(type: type, minize: false))
                      : sectionBloc.add(const SetSection(section: type));
                  break;
                }
              case LogicalKeyboardKey.digit4:
                {
                  const type = SectionType.event;
                  final isMinize = sectionBloc.state.sectionMinize[type]!;
                  isMinize && type == sectionBloc.state.section
                      ? sectionBloc.add(
                          const SectionMinizeToggled(type: type, minize: false))
                      : sectionBloc.add(const SetSection(section: type));
                  break;
                }
              case LogicalKeyboardKey.keyL:
                {
                  sectionBloc.add(const ExtensionToggled(
                    type: ExtensionType.layer,
                  ));
                  break;
                }
              case LogicalKeyboardKey.keyH:
                {
                  sectionBloc.add(const ExtensionToggled(
                    type: ExtensionType.history,
                  ));
                  break;
                }
              case LogicalKeyboardKey.keyI:
                {
                  sectionBloc.add(const ExtensionToggled(
                    type: ExtensionType.setting,
                  ));
                  break;
                }
              case LogicalKeyboardKey.keyP:
                {
                  sectionBloc.add(const ExtensionToggled(
                    type: ExtensionType.palette,
                  ));
                  break;
                }
              case LogicalKeyboardKey.keyZ:
                {
                  final historyBloc = context.read<HistoryBloc>();
                  if (historyBloc.state.captureManager.canUndo()) {
                    historyBloc.add(const UndoEvent());
                    final scrollController = historyBloc.state.scrollController;
                    if (!(scrollController.position.atEdge &&
                        scrollController.position.pixels == 0)) {
                      scrollController
                          .jumpTo(scrollController.position.pixels - 45);
                    }
                  }
                  break;
                }
              case LogicalKeyboardKey.keyY:
                {
                  final historyBloc = context.read<HistoryBloc>();
                  if (historyBloc.state.captureManager.canRedo()) {
                    historyBloc.add(const RedoEvent());
                    final scrollController = historyBloc.state.scrollController;
                    if (!(scrollController.position.atEdge &&
                        scrollController.position.pixels != 0)) {
                      scrollController
                          .jumpTo(scrollController.position.pixels + 45);
                    }
                  }
                  break;
                }
              default:
                {
                  toolbarBloc.add(const ToolToogled(
                      type: ToolType.rectangleTool, enabled: false));
                  break;
                }
            }
          } else {
            switch (logicalKey) {
              case LogicalKeyboardKey.f2:
                {
                  toolbarBloc.add(const ShortcutMenuEvent());
                }
              case LogicalKeyboardKey.f8:
                {
                  toolbarBloc.add(const ToolClearEvent());
                  break;
                }
              case LogicalKeyboardKey.f9:
                {
                  toolbarBloc.add(const ToolConfigEvent());
                  break;
                }
              case LogicalKeyboardKey.delete:
                {
                  _delete(context);
                  break;
                }
              case LogicalKeyboardKey.keyE:
                {
                  toolbarBloc.add(const ToolToogled(
                      type: ToolType.eraseTool, enabled: false));
                  break;
                }
              case LogicalKeyboardKey.keyW:
              case LogicalKeyboardKey.keyA:
              case LogicalKeyboardKey.keyS:
              case LogicalKeyboardKey.keyD:
                {
                  final itemBloc = context.read<ItemBloc>();
                  final historyBloc = context.read<HistoryBloc>();
                  final actionService = ActionService<ActionModelType>(
                      actionType: ActionType.moveItem,
                      data: {
                        ActionModelType.mapPieces: stageBloc.clonePieces(),
                        ActionModelType.mapEvents: stageBloc.cloneEvents(),
                      },
                      change: (data) {
                        final pieces = data![ActionModelType.mapPieces]
                            as HashMap<String, MapPieceItem>;
                        final events = data[ActionModelType.mapEvents]
                            as HashMap<String, MapEventItem>;
                        stageBloc.add(UpdateStageState(
                            stageState: stageBloc.state
                                .copyWith(events: events, pieces: pieces)));
                        itemBloc.add(const ItemStoreUpdated());
                        historyBloc.add(const UpdateIndexEvent());
                      });
                  historyBloc.add(CaptureState(state: actionService));
                  break;
                }
              case LogicalKeyboardKey.keyJ:
              case LogicalKeyboardKey.keyK:
              case LogicalKeyboardKey.keyN:
              case LogicalKeyboardKey.keyM:
                {
                  final itemBloc = context.read<ItemBloc>();
                  final historyBloc = context.read<HistoryBloc>();
                  final actionService = ActionService<ActionModelType>(
                      actionType: ActionType.islandScale,
                      data: {
                        ActionModelType.mapPieces: stageBloc.clonePieces(),
                        ActionModelType.mapEvents: stageBloc.cloneEvents(),
                      },
                      change: (data) {
                        final pieces = data![ActionModelType.mapPieces]
                            as HashMap<String, MapPieceItem>;
                        final events = data[ActionModelType.mapEvents]
                            as HashMap<String, MapEventItem>;
                        stageBloc.add(UpdateStageState(
                            stageState: stageBloc.state
                                .copyWith(events: events, pieces: pieces)));
                        itemBloc.add(const ItemStoreUpdated());
                        historyBloc.add(const UpdateIndexEvent());
                      });
                  historyBloc.add(CaptureState(state: actionService));
                  break;
                }
              case LogicalKeyboardKey.keyZ:
              case LogicalKeyboardKey.keyX:
                {
                  final itemBloc = context.read<ItemBloc>();
                  final historyBloc = context.read<HistoryBloc>();
                  final actionService = ActionService<ActionModelType>(
                      actionType: ActionType.islandRotate,
                      data: {
                        ActionModelType.mapPieces: stageBloc.clonePieces(),
                        ActionModelType.mapEvents: stageBloc.cloneEvents(),
                      },
                      change: (data) {
                        final pieces = data![ActionModelType.mapPieces]
                            as HashMap<String, MapPieceItem>;
                        final events = data[ActionModelType.mapEvents]
                            as HashMap<String, MapEventItem>;
                        stageBloc.add(UpdateStageState(
                            stageState: stageBloc.state
                                .copyWith(events: events, pieces: pieces)));
                        itemBloc.add(const ItemStoreUpdated());
                        historyBloc.add(const UpdateIndexEvent());
                      });
                  historyBloc.add(CaptureState(state: actionService));
                  break;
                }
              case LogicalKeyboardKey.keyF:
                {
                  final selectedList =
                      context.read<SelectedBloc>().state.selectedList;
                  for (final id in selectedList) {
                    final itemProfile =
                        context.read<ItemBloc>().state.itemStore[id];
                    if (itemProfile != null) {
                      if (!itemProfile.isEvent) {
                        final piece = stageBloc.state.pieces[id]!;
                        piece.isArtFlipped = !piece.isArtFlipped;
                      }
                    }
                  }
                  context.read<ItemBloc>().add(const ItemStoreUpdated());
                  final itemBloc = context.read<ItemBloc>();
                  final historyBloc = context.read<HistoryBloc>();
                  final actionService = ActionService<ActionModelType>(
                      actionType: ActionType.islandArtFlip,
                      data: {
                        ActionModelType.mapPieces: stageBloc.clonePieces(),
                        ActionModelType.mapEvents: stageBloc.cloneEvents(),
                      },
                      change: (data) {
                        final pieces = data![ActionModelType.mapPieces]
                            as HashMap<String, MapPieceItem>;
                        final events = data[ActionModelType.mapEvents]
                            as HashMap<String, MapEventItem>;
                        stageBloc.add(UpdateStageState(
                            stageState: stageBloc.state
                                .copyWith(events: events, pieces: pieces)));
                        itemBloc.add(const ItemStoreUpdated());
                        historyBloc.add(const UpdateIndexEvent());
                      });
                  historyBloc.add(CaptureState(state: actionService));
                  break;
                }
              case LogicalKeyboardKey.space:
                {
                  toolbarBloc.add(const ToolToogled(
                      type: ToolType.panTool, enabled: false));
                  break;
                }
              default:
                break;
            }
          }
        },
        child: child);
  }
}
