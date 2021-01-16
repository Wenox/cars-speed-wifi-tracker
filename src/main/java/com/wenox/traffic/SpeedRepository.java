package com.wenox.traffic;

import com.wenox.traffic.domain.Speed;
import org.springframework.data.repository.CrudRepository;
import org.springframework.stereotype.Repository;

@Repository
public interface SpeedRepository extends CrudRepository<Speed, Integer> {
}
