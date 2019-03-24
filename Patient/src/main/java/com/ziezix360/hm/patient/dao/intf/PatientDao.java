package com.ziezix360.hm.patient.dao.intf;

import com.ziezix360.hm.patient.dao.model.Patient;
import com.ziezix360hm.Dao;

import java.util.List;

public interface PatientDao extends Dao<Patient> {

        Patient getByNationalId(int nationalId);
        List<Patient> getByName(String name);
        List<Patient> getBySurname(String surname);
}
