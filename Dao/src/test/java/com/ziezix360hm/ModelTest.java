package com.ziezix360hm;

import javax.persistence.*;
import java.util.Date;

@Entity
@Table(name = "PATIENTS", schema = "HM")
public class ModelTest extends DaoModel {

    @Column(name = "ID") @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    int id;
    @Column(name = "INSURANCE_NUMBER")
    int insuranceNumber;
    @Column(name = "NATIONAL_ID")
    int nationalId;
    @Column(name = "NAME")
    String name;
    @Column(name = "DATE_OF_BIRTH")
    Date dateofBirth;
    @Column(name = "MOTHER_NAME")
    String motherName;

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

    public Date getDateofBirth() {
        return dateofBirth;
    }

    public void setDateofBirth(Date dateofBirth) {
        this.dateofBirth = dateofBirth;
    }

    public String getMotherName() {
        return motherName;
    }

    public void setMotherName(String motherName) {
        this.motherName = motherName;
    }

    @Override
    public int getId() {
        return 0;
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append("[")
                .append(id)
                .append(", ")
                .append(nationalId)
                .append(", ")
                .append(insuranceNumber)
                .append(", ")
                .append(name)
                .append(", ")
                .append(dateofBirth)
                .append(", ")
                .append(motherName)
                .append("]");
        return sb.toString();

    }
}
