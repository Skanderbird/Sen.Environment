part of 'miscellaneous_task_bloc.dart';

@immutable
sealed class MiscellaneousTaskEvent extends Equatable {
  const MiscellaneousTaskEvent();

  @override
  List<Object?> get props => [];
}

class BackupConfigurationRequested extends MiscellaneousTaskEvent {
  const BackupConfigurationRequested();
}

class DownloadScriptRequested extends MiscellaneousTaskEvent {
  const DownloadScriptRequested({required this.settingsBloc});

  final SettingsBloc settingsBloc;

  @override
  List<Object?> get props => [settingsBloc];
}
