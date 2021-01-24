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
      messageRepository.save(new Info("ZAPNIJ PASY"));
      messageRepository.save(new Info("DRIVE SAFE"));
      messageRepository.save(new Info("TRZYMAJ SIE PRAWEGO PASA"));
      messageRepository.save(new Info("PAMIETAJ O KORYTARZU ZYCIA"));
      messageRepository.save(new Info("UWAGA! WYPADKI"));
      messageRepository.save(new Info("KONTROLA PREDKOSCI"));
      messageRepository.save(new Info("REMONT 3 KM"));
      messageRepository.save(new Info("OBJAZD AUTOSTRADA A4"));
      messageRepository.save(new Info("INFORMACJA DROGOWA 19-111"));
      messageRepository.save(new Info("KONTROLA GRANICZNA"));
      messageRepository.save(new Info("STOSUJ JAZDE NA SUWAK"));
    };
  }
}
