import 'package:flutter/material.dart';

class AnimatedFloating extends StatefulWidget {
  const AnimatedFloating({super.key, required this.child});

  final Widget child;

  @override
  State<AnimatedFloating> createState() => _AnimatedFloatingState();
}

class _AnimatedFloatingState extends State<AnimatedFloating>
    with SingleTickerProviderStateMixin {
  late AnimationController _controller;
  late Animation<double> _steamAnimation;
  late Animation<double> _scaleAnimation;

  @override
  void initState() {
    super.initState();
    _controller = AnimationController(
      duration: const Duration(milliseconds: 600),
      vsync: this,
    )..repeat(reverse: true);
    _steamAnimation = Tween<double>(
      begin: 0,
      end: -10,
    ).animate(CurvedAnimation(parent: _controller, curve: Curves.easeInOut));
    _scaleAnimation = Tween<double>(
      begin: 1,
      end: 1.05,
    ).animate(CurvedAnimation(parent: _controller, curve: Curves.easeInOut));
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return AnimatedBuilder(
      animation: _controller,
      builder: (context, child) {
        final steamValue = _steamAnimation.value;
        final scaleValue = _scaleAnimation.value;
        return Transform(
          transform:
              Matrix4.identity()
                ..translate(0.0, steamValue)
                ..scale(scaleValue),
          alignment: Alignment.bottomCenter,
          child: widget.child,
        );
      },
    );
  }
}
