part of 'add_option_bloc.dart';

@immutable
sealed class AddOptionState extends Equatable {
  const AddOptionState();

  @override
  List<Object?> get props => [];
}

final class AddOptionInitial extends AddOptionState {
  const AddOptionInitial();
}

final class ExportLogState extends AddOptionState {
  final String value;
  const ExportLogState({
    required this.value,
  });

  @override
  List<Object?> get props => [value];
}

final class NoneOptionState extends AddOptionState {
  const NoneOptionState();
}
