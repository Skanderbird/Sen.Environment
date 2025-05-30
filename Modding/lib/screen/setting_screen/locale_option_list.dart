import 'package:flutter/material.dart';
import 'package:flutter_bloc/flutter_bloc.dart';
import '../../bloc/settings_bloc/settings_bloc.dart';
import '../../extension/context.dart';

class LocaleOptionList extends StatelessWidget {
  const LocaleOptionList({super.key});

  Map<String, String> locOf(BuildContext context) {
    return {
      'en': context.los.en,
      'vi': context.los.vi,
      'es': context.los.es,
      'ru': context.los.ru,
      'fr': context.los.fr,
      'de': context.los.de,
    };
  }

  @override
  Widget build(BuildContext context) {
    final locales = locOf(context);
    return Column(
      mainAxisSize: MainAxisSize.min,
      children: [
        ...locales.entries.map(
          (entry) => RadioListTile<String>(
            contentPadding: EdgeInsets.zero,
            title: Text(
              entry.value,
              style: Theme.of(context).textTheme.labelLarge,
            ),
            value: entry.key,
            groupValue: context.watch<SettingsBloc>().state.locale,
            onChanged: (String? locale) async {
              if (locale == null) {
                return;
              }
              context.read<SettingsBloc>().add(SetLocale(locale));
            },
          ),
        ),
      ],
    );
  }
}
