import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:flutter_bloc/flutter_bloc.dart';
import 'package:material_symbols_icons/symbols.dart';
import '../../bloc/miscellaneous_task_bloc/miscellaneous_task_bloc.dart';
import '../../bloc/settings_bloc/settings_bloc.dart';
import '../../extension/context.dart';
import '../../extension/platform.dart';
import '../../i18n/app_localizations.dart';
import 'backup_setting.dart';
import '../../service/ui_helper.dart';
import 'register_context_menu.dart';

class MiscellaneousScreen extends StatelessWidget {
  const MiscellaneousScreen({super.key});

  void _onBackup(BuildContext context) {
    Navigator.of(
      context,
    ).push(MaterialPageRoute(builder: (context) => const BackupSetting()));
  }

  @override
  Widget build(BuildContext context) {
    final los = AppLocalizations.of(context)!;
    return BlocProvider<MiscellaneousTaskBloc>(
      create: (context) => MiscellaneousTaskBloc(),
      child: Builder(
        builder: (context) {
          return BlocListener<MiscellaneousTaskBloc, MiscellaneousTaskState>(
            listener: (context, state) async {
              if (state is ScriptDownloaded) {
                await UIHelper.showSimpleDialog(
                  context: context,
                  title: los.done,
                  content: los.script_has_been_downloaded,
                );
              } else if (state is ScriptDownloadFailed) {
                ScaffoldMessenger.of(context).showSnackBar(
                  SnackBar(
                    content: Text(
                      state.error,
                      softWrap: true,
                      overflow: TextOverflow.visible,
                    ),
                    width: UIHelper.ofDesktop(builder: () => 400.0),
                    behavior: SnackBarBehavior.floating,
                    duration: const Duration(milliseconds: 1500),
                    shape: RoundedRectangleBorder(
                      borderRadius: BorderRadius.circular(10.0),
                    ),
                  ),
                );
              }
            },
            child:
                (() {
                  if (CurrentPlatform.isDesktop) {
                    return DesktopLayout(onBackup: () => _onBackup(context));
                  }
                  return MobileLayout(onBackup: () => _onBackup(context));
                })(),
          );
        },
      ),
    );
  }
}

class BackupConfiguration extends StatelessWidget {
  const BackupConfiguration({super.key, required this.onBackup});

  final void Function() onBackup;

  @override
  Widget build(BuildContext context) {
    final los = context.los;
    return Card(
      shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(16.0)),
      elevation: 4,
      child: ListTile(
        leading: Icon(
          Symbols.backup,
          size: 28,
          color: Colors.lightBlueAccent.withValues(alpha: 0.8),
        ),
        title: Text(
          los.backup_configuration,
          style: Theme.of(context).textTheme.titleMedium,
        ),
        subtitle: Text(
          los.backup_configuration_description,
          style: Theme.of(context).textTheme.bodyMedium,
        ),
        onTap: onBackup,
        trailing: const Icon(
          Symbols.arrow_forward,
          size: 24.0,
          color: Colors.grey,
        ),
      ),
    );
  }

  @override
  void debugFillProperties(DiagnosticPropertiesBuilder properties) {
    super.debugFillProperties(properties);
    properties.add(
      ObjectFlagProperty<void Function()>.has('onBackup', onBackup),
    );
  }
}

class CustomScriptInstallButton extends StatelessWidget {
  const CustomScriptInstallButton({super.key});
  @override
  Widget build(BuildContext context) {
    final los = context.los;
    return BlocBuilder<MiscellaneousTaskBloc, MiscellaneousTaskState>(
      builder: (context, state) {
        return Card(
          shape: RoundedRectangleBorder(
            borderRadius: BorderRadius.circular(16.0),
          ),
          elevation: 4.0,
          child: ListTile(
            leading: Icon(
              Symbols.download_2,
              size: 28.0,
              color: Colors.green.withValues(alpha: 0.8),
            ),
            title: Text(
              los.download_script,
              style: Theme.of(context).textTheme.titleMedium,
            ),
            subtitle: Text(
              los.download_script_description,
              style: Theme.of(context).textTheme.bodyMedium,
            ),
            onTap:
                CurrentPlatform.isAndroid
                    ? () => BlocProvider.of<MiscellaneousTaskBloc>(context).add(
                      DownloadScriptRequested(
                        settingsBloc: context.read<SettingsBloc>(),
                      ),
                    )
                    : null,
            enabled: CurrentPlatform.isAndroid,
            trailing:
                state is DownloadingScript
                    ? const SizedBox(
                      width: 24,
                      height: 24,
                      child: CircularProgressIndicator.adaptive(),
                    )
                    : const _DownloadableButton(),
          ),
        );
      },
    );
  }
}

class _DownloadableButton extends StatelessWidget {
  const _DownloadableButton();

  @override
  Widget build(BuildContext context) {
    return CurrentPlatform.isMobile
        ? const Icon(Symbols.arrow_downward, size: 24.0, color: Colors.grey)
        : Tooltip(
          message: context.los.not_specified,
          child: const Icon(Symbols.dangerous, size: 24.0, color: Colors.red),
        );
  }
}

class MobileLayout extends StatelessWidget {
  const MobileLayout({super.key, required this.onBackup});

  final void Function() onBackup;

  @override
  Widget build(BuildContext context) {
    return Padding(
      padding: const EdgeInsets.symmetric(horizontal: 8.0),
      child: Column(
        spacing: 4.0,
        children: [
          BackupConfiguration(onBackup: onBackup),
          const CustomScriptInstallButton(),
        ],
      ),
    );
  }

  @override
  void debugFillProperties(DiagnosticPropertiesBuilder properties) {
    super.debugFillProperties(properties);
    properties.add(
      ObjectFlagProperty<void Function()>.has('onBackup', onBackup),
    );
  }
}

class DesktopLayout extends StatelessWidget {
  const DesktopLayout({super.key, required this.onBackup});

  final void Function() onBackup;

  @override
  Widget build(BuildContext context) {
    return SafeArea(
      child: ListView(
        padding: const EdgeInsets.all(8.0),
        children: [
          Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            mainAxisSize: MainAxisSize.min,
            children: [
              CustomDesktopBackupCard(onBackup: onBackup),
              const CustomDesktopDownloadCard(),
              const CustomContextMenuRegisterCard(),
            ],
          ),
        ],
      ),
    );
  }

  @override
  void debugFillProperties(DiagnosticPropertiesBuilder properties) {
    super.debugFillProperties(properties);
    properties.add(
      ObjectFlagProperty<void Function()>.has('onBackup', onBackup),
    );
  }
}

class CustomDesktopBackupCard extends StatelessWidget {
  const CustomDesktopBackupCard({super.key, required this.onBackup});

  final void Function() onBackup;

  @override
  Widget build(BuildContext context) {
    final los = context.los;
    return Card(
      elevation: 4,
      shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(16.0)),
      child: Padding(
        padding: const EdgeInsets.symmetric(vertical: 12.0, horizontal: 20.0),
        child: Row(
          mainAxisAlignment: MainAxisAlignment.spaceBetween,
          children: [
            Row(
              spacing: 10.0,
              children: [
                Icon(
                  Symbols.backup,
                  size: 28,
                  color: Colors.lightBlueAccent.withValues(alpha: 0.8),
                ),
                Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  spacing: 4.0,
                  children: [
                    Text(
                      los.backup_configuration,
                      style: Theme.of(context).textTheme.titleMedium,
                    ),
                    Text(
                      los.backup_configuration_description,
                      style: Theme.of(context).textTheme.bodyMedium,
                    ),
                  ],
                ),
              ],
            ),
            FilledButton.icon(
              onPressed: onBackup,
              label: Text(los.backup_configuration),
            ),
          ],
        ),
      ),
    );
  }

  @override
  void debugFillProperties(DiagnosticPropertiesBuilder properties) {
    super.debugFillProperties(properties);
    properties.add(
      ObjectFlagProperty<void Function()>.has('onBackup', onBackup),
    );
  }
}

class CustomDesktopDownloadCard extends StatelessWidget {
  const CustomDesktopDownloadCard({super.key});

  @override
  Widget build(BuildContext context) {
    final los = context.los;
    return Card(
      elevation: 4,
      shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(16.0)),
      child: Padding(
        padding: const EdgeInsets.symmetric(vertical: 12.0, horizontal: 20.0),
        child: Row(
          mainAxisAlignment: MainAxisAlignment.spaceBetween,
          children: [
            Row(
              spacing: 12.0,
              children: [
                Icon(
                  Symbols.download_2,
                  size: 28.0,
                  color: Colors.green.withValues(alpha: 0.8),
                ),
                Column(
                  spacing: 4.0,
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Text(
                      los.download_script,
                      style: Theme.of(context).textTheme.titleMedium,
                    ),
                    Text(
                      los.download_script_description,
                      style: Theme.of(context).textTheme.bodyMedium,
                    ),
                  ],
                ),
              ],
            ),
            FilledButton.icon(
              onPressed: null,
              label: Text(los.download_script),
            ),
          ],
        ),
      ),
    );
  }
}

class CustomContextMenuRegisterCard extends StatelessWidget {
  const CustomContextMenuRegisterCard({super.key});

  @override
  Widget build(BuildContext context) {
    return Card(
      elevation: 4,
      shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(16.0)),
      child: Padding(
        padding: const EdgeInsets.symmetric(vertical: 12.0, horizontal: 20.0),
        child: Row(
          mainAxisAlignment: MainAxisAlignment.spaceBetween,
          children: [
            Row(
              spacing: 10.0,
              children: [
                Icon(
                  Symbols.terminal,
                  size: 28,
                  color: Colors.lightBlueAccent.withValues(alpha: 0.8),
                ),
                Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  spacing: 4.0,
                  children: [
                    Text(
                      'Context Menu Register',
                      style: Theme.of(context).textTheme.titleMedium,
                    ),
                    Text(
                      'Register legacy context menu',
                      style: Theme.of(context).textTheme.bodyMedium,
                    ),
                  ],
                ),
              ],
            ),
            FilledButton.icon(
              onPressed: () {
                Navigator.of(context).push(
                  MaterialPageRoute(
                    builder: (context) {
                      return const RegisterContextMenu();
                    },
                  ),
                );
              },
              label: const Text('Register'),
            ),
          ],
        ),
      ),
    );
  }
}
