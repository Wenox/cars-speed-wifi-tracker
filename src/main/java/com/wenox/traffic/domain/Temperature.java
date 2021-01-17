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
public class Temperature {

  @Id
  @GeneratedValue(strategy = GenerationType.AUTO)
  private Integer id;

  @Column(name = "temperature_sensor_1", nullable = false, precision = 2)
  private Double value1;

  @Column(name = "temperature_sensor_2", nullable = false, precision = 2)
  private Double value2;

  @Column(name = "registered_time", nullable = false)
  private final LocalDateTime localDateTime = LocalDateTime.now();

  public Temperature(Double value1, Double value2) {
    this.value1 = value1;
    this.value2 = value2;
  }
}
