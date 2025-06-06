import 'package:flutter/material.dart';
import 'package:flutter_bloc/flutter_bloc.dart';
import '../../bloc/settings_bloc/settings_bloc.dart';
import '../../extension/context.dart';

class NotificationOptionList extends StatelessWidget {
  const NotificationOptionList({super.key});

  Future<void> _onNotificationChanged(BuildContext context, bool? value) async {
    if (value == null) {
      return;
    }
    context.read<SettingsBloc>().add(SetNotification(sendNotification: value));
  }

  @override
  Widget build(BuildContext context) {
    final isEnabled = context.watch<SettingsBloc>().state.sendNotification;
    return Column(
      mainAxisSize: MainAxisSize.min,
      children: [
        RadioListTile<bool>(
          contentPadding: EdgeInsets.zero,
          title: Text(
            context.los.enable,
            style: Theme.of(context).textTheme.labelLarge,
          ),
          value: true,
          groupValue: isEnabled,
          onChanged: (value) => _onNotificationChanged(context, value),
        ),
        RadioListTile<bool>(
          contentPadding: EdgeInsets.zero,
          title: Text(
            context.los.disable,
            style: Theme.of(context).textTheme.labelLarge,
          ),
          value: false,
          groupValue: isEnabled,
          onChanged: (value) => _onNotificationChanged(context, value),
        ),
      ],
    );
  }
}
