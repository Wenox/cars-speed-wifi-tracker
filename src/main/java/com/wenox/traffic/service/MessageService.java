package com.wenox.traffic.service;

import com.wenox.traffic.domain.Message;
import com.wenox.traffic.repository.MessageRepository;
import java.util.List;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Service;

@Service
public class MessageService {

  private final MessageRepository messageRepository;

  private final String defaultMessage;

  private int counter = 0;

  public MessageService(@Value("${app.defaultMessage}") String defaultMessage,
                        MessageRepository messageRepository) {
    this.defaultMessage = defaultMessage;
    this.messageRepository = messageRepository;
  }

  public Message add(String message) {
    return messageRepository.save(new Message(message));
  }

  public String getNextMessage() {
    List<Message> messages = messageRepository.findAll();

    if (messages.isEmpty()) {
      return defaultMessage;
    }

    Message message = messages.get(counter % messages.size());

    counter++;

    return message.getMessage();
  }
}
