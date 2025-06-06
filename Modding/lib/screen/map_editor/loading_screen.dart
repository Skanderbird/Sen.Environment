import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:loading_animation_widget/loading_animation_widget.dart';
import '../../extension/context.dart';
import '../../widget/background.dart';

class LoadingScreen extends StatefulWidget {
  const LoadingScreen({super.key});

  @override
  State<LoadingScreen> createState() => _LoadingScreenState();
}

class _LoadingScreenState extends State<LoadingScreen>
    with SingleTickerProviderStateMixin {
  late AnimationController _shimmerController;
  late Animation<double> _shimmerAnimation;

  @override
  void initState() {
    super.initState();
    _shimmerController = AnimationController(
      vsync: this,
      duration: const Duration(milliseconds: 1500),
    )..repeat();

    _shimmerAnimation = Tween<double>(begin: -2.0, end: 2.0).animate(
      CurvedAnimation(parent: _shimmerController, curve: Curves.easeInOut),
    );
  }

  @override
  void dispose() {
    _shimmerController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Background(
      child: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          spacing: 20.0,
          children: [
            CircularProgressIndicator(
              strokeWidth: 4.5,
              strokeAlign: 1.0,
              color: Theme.of(context).colorScheme.primary,
            ),
            ShimmerText(
              controller: _shimmerController,
              animation: _shimmerAnimation,
            ),
          ],
        ),
      ),
    );
  }
}

class ShimmerText extends StatelessWidget {
  const ShimmerText({
    super.key,
    required this.controller,
    required this.animation,
  });

  final AnimationController controller;

  final Animation<double> animation;

  @override
  Widget build(BuildContext context) {
    final textStyle = Theme.of(
      context,
    ).textTheme.labelLarge!.copyWith(fontWeight: FontWeight.bold);
    return AnimatedBuilder(
      animation: controller,
      builder: (context, child) {
        return ShaderMask(
          shaderCallback: (Rect bounds) {
            return LinearGradient(
              colors: [
                context.isDarkMode
                    ? Colors.white.withValues(alpha: 0.3)
                    : Theme.of(
                      context,
                    ).colorScheme.secondary.withValues(alpha: 0.82),
                Theme.of(context).colorScheme.primary,
                context.isDarkMode
                    ? Colors.white.withValues(alpha: 0.3)
                    : Theme.of(
                      context,
                    ).colorScheme.secondary.withValues(alpha: 0.82),
              ],
              stops: const [0.0, 0.5, 1.0],
              begin: Alignment(animation.value, 0),
              end: Alignment(animation.value + 1.5, 0),
            ).createShader(bounds);
          },
          blendMode: BlendMode.srcIn,
          child: Row(
            crossAxisAlignment: CrossAxisAlignment.end,
            mainAxisAlignment: MainAxisAlignment.center,
            spacing: 4.0,
            children: [
              Text(context.los.loading_map_resources, style: textStyle),
              LoadingAnimationWidget.waveDots(
                color: Theme.of(context).colorScheme.primary,
                size: textStyle.fontSize!,
              ),
            ],
          ),
        );
      },
    );
  }

  @override
  void debugFillProperties(DiagnosticPropertiesBuilder properties) {
    super.debugFillProperties(properties);
    properties.add(
      DiagnosticsProperty<AnimationController>('controller', controller),
    );
    properties.add(
      DiagnosticsProperty<Animation<double>>('animation', animation),
    );
  }
}
