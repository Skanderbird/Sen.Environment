import 'package:bloc/bloc.dart';
import 'package:equatable/equatable.dart';
import 'package:meta/meta.dart';
import 'package:sen/model/message.dart';

part 'message_event.dart';
part 'message_state.dart';

class MessageBloc extends Bloc<MessageEvent, MessageState> {
  MessageBloc() : super(const MessageInitialState(messages: [])) {
    on<AddMessage>(_addMessage);
    on<ClearMessage>(_clearMessage);
  }

  void _addMessage(
    AddMessage event,
    Emitter<MessageState> emit,
  ) {
    emit(MessageAddState(messages: [...state.messages, event.message]));
  }

  void _clearMessage(
    ClearMessage event,
    Emitter<MessageState> emit,
  ) {
    emit(const MessageClearState(messages: []));
  }
}
