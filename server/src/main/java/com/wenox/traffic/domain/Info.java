package com.wenox.traffic.domain;

import java.time.LocalDateTime;
import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.GeneratedValue;
import javax.persistence.GenerationType;
import javax.persistence.Id;
import lombok.Data;
import lombok.NoArgsConstructor;

@Entity
@Data
@NoArgsConstructor
public class Info {

  @Id
  @GeneratedValue(strategy = GenerationType.AUTO)
  private Integer id;

  @Column(name = "message", nullable = false)
  private String message;

  @Column(name = "registered_time", nullable = false)
  private final LocalDateTime localDateTime = LocalDateTime.now();

  public Info(String message) {
    this.message = message;
  }
}