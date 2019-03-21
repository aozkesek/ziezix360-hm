package com.ziezix360.hm.patient.dao.model;

import com.ziezix360hm.DaoModel;

import javax.persistence.*;
import java.util.Date;

@Entity
@Table(name = "PATIENTS", schema = "HM")
public class Patient extends DaoModel {

    @Column(name = "ID") @Id @GeneratedValue(strategy = GenerationType.IDENTITY)
    int id;
    @Column(name = "INSURANCE_NUMBER")
    int insuranceNumber;
    @Column(name = "NATIONAL_ID")
    int nationalId;
    @Column(name = "NAME")
    String name;
    @Column(name = "SURNAME")
    String surname;
    @Column(name = "DATE_OF_BIRTH")
    Date dateofBirth;
    @Column(name = "PLACE_OF_BIRTH")
    String placeofBirth;
    @Column(name = "FATHER_NAME")
    String fatherName;
    @Column(name = "MOTHER_NAME")
    String motherName;

    public int getId() {
        return id;
    }

    public void setId(int id) {
        this.id = id;
    }

    public int getInsuranceNumber() {
        return insuranceNumber;
    }

    public void setInsuranceNumber(int insuranceNumber) {
        this.insuranceNumber = insuranceNumber;
    }

    public int getNationalId() {
        return nationalId;
    }

    public void setNationalId(int nationalId) {
        this.nationalId = nationalId;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getSurname() {
        return surname;
    }

    public void setSurname(String surname) {
        this.surname = surname;
    }

    public Date getDateofBirth() {
        return dateofBirth;
    }

    public void setDateofBirth(Date dateofBirth) {
        this.dateofBirth = dateofBirth;
    }

    public String getPlaceofBirth() {
        return placeofBirth;
    }

    public void setPlaceofBirth(String placeofBirth) {
        this.placeofBirth = placeofBirth;
    }

    public String getFatherName() {
        return fatherName;
    }

    public void setFatherName(String fatherName) {
        this.fatherName = fatherName;
    }

    public String getMotherName() {
        return motherName;
    }

    public void setMotherName(String motherName) {
        this.motherName = motherName;
    }

}
