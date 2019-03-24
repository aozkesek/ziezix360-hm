package com.ziezix360.hm.patient.dao.impl;

import com.ziezix360.hm.patient.dao.intf.PatientDao;
import com.ziezix360.hm.patient.dao.model.Patient;
import com.ziezix360hm.DaoImpl;
import org.springframework.stereotype.Component;

import javax.persistence.criteria.CriteriaBuilder;
import javax.persistence.criteria.Predicate;
import javax.persistence.criteria.Root;
import java.util.List;

@Component
public class PatientDaoImpl extends DaoImpl<Patient> implements PatientDao {

    static final String GetByName = "GETBYNAME";
    static final String GetBySurname = "GETBYSURNAME";
    static final String GetByNationalId = "GETBYNATIONALID";

    @Override
    public Patient getByNationalId(int nationalId) {
        return query(GetByNationalId, new Patient(nationalId)).get(0);
    }

    @Override
    public List<Patient> getByName(String name) {
        return query(GetByName, new Patient(name, null));
    }

    @Override
    public List<Patient> getBySurname(String surname) {
        return query(GetBySurname, new Patient(null, surname));
    }

    @Override
    public Class getModelClass() {
        return Patient.class;
    }

    @Override
    public Predicate buildCriteriaQuery(CriteriaBuilder criteriaBuilder,
                                        Root<Patient> root, String name,
                                        Patient patient) {
        if (name == null)
            // this mean return ALL
            return root.isNotNull();

        switch (name) {
            case GetByName:
                return criteriaBuilder.equal(root.get("name"), patient.getName());
            case GetBySurname:
                return criteriaBuilder.equal(root.get("surname"), patient.getSurname());
            case GetByNationalId:
                return criteriaBuilder.equal(root.get("nationalId"), patient.getNationalId());

        }

        return root.isNull();

    }
}
