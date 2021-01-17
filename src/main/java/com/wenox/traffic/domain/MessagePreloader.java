package com.wenox.traffic.domain;

import com.wenox.traffic.repository.MessageRepository;
import org.springframework.boot.CommandLineRunner;
import org.springframework.boot.autoconfigure.condition.ConditionalOnProperty;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.context.annotation.Profile;

@Configuration
@Profile("!test")
@ConditionalOnProperty(value = "spring.jpa.hibernate.ddl-auto", havingValue = "create")
public class MessagePreloader {

  @Bean
  public CommandLineRunner loadMessages(MessageRepository messageRepository) {
    return args -> {
      messageRepository.save(new Message("ZAPNIJ PASY"));
      messageRepository.save(new Message("DRIVE SAFE"));
      messageRepository.save(new Message("TRZYMAJ SIE PRAWEGO PASA"));
      messageRepository.save(new Message("PAMIETAJ O KORYTARZU ZYCIA"));
      messageRepository.save(new Message("UWAGA! WYPADKI"));
      messageRepository.save(new Message("REMONT 3 KM"));
      messageRepository.save(new Message("OBJAZD AUTOSTRADA A4"));
      messageRepository.save(new Message("INFORMACJA DROGOWA 19-111"));
      messageRepository.save(new Message("KONTROLA GRANICZNA"));
      messageRepository.save(new Message("KONTROLA PREDKOSCI"));
      messageRepository.save(new Message("STOSUJ JAZDE NA ZAMEK"));
    };
  }
}
