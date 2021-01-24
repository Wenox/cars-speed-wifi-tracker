package com.wenox.traffic.service;

import com.wenox.traffic.domain.Info;
import com.wenox.traffic.repository.MessageRepository;
import java.util.List;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Service;

@Service
public class InfoService {

  private final MessageRepository messageRepository;

  private final String defaultMessage;

  private int counter = 0;

  public InfoService(@Value("${app.defaultMessage}") String defaultMessage,
                     MessageRepository messageRepository) {
    this.defaultMessage = defaultMessage;
    this.messageRepository = messageRepository;
  }

  public Info add(String message) {
    return messageRepository.save(new Info(message));
  }

  public String getNextMessage() {
    List<Info> infos = messageRepository.findAll();

    if (infos.isEmpty()) {
      return defaultMessage;
    }

    Info info = infos.get(counter % infos.size());

    counter++;

    return info.getMessage();
  }
}
