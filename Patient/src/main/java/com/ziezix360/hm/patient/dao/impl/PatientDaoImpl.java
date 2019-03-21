package com.ziezix360.hm.patient.dao.impl;

import com.ziezix360.hm.patient.dao.intf.PatientDao;
import com.ziezix360.hm.patient.dao.model.Patient;
import com.ziezix360hm.DaoImpl;
import org.springframework.stereotype.Component;

@Component
public class PatientDaoImpl extends DaoImpl<Patient> implements PatientDao {
    @Override
    public Class getModelClass() {
        return Patient.class;
    }
}
