import 'package:flutter/material.dart';
import 'package:material_symbols_icons/symbols.dart';
import 'package:sen/extension/l10n.dart';

class EnumerationBar extends StatelessWidget {
  const EnumerationBar({
    super.key,
    required this.onSelect,
  });

  final void Function() onSelect;

  @override
  Widget build(BuildContext context) {
    return Padding(
      padding: const EdgeInsets.symmetric(horizontal: 8.0),
      child: SizedBox(
        height: 46.0,
        width: double.infinity,
        child: FloatingActionButton(
          onPressed: onSelect,
          child: Row(
            crossAxisAlignment: CrossAxisAlignment.center,
            mainAxisSize: MainAxisSize.min,
            spacing: 10.0,
            children: [
              const Icon(Symbols.arrow_drop_down),
              Text(context.los.select_an_option),
            ],
          ),
        ),
      ),
    );
  }
}
