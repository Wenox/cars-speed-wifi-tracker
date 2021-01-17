package com.wenox.traffic.repository;

import com.wenox.traffic.domain.Temperature;
import org.springframework.data.repository.CrudRepository;
import org.springframework.stereotype.Repository;

@Repository
public interface TemperatureRepository extends CrudRepository<Temperature, Integer> {
}
