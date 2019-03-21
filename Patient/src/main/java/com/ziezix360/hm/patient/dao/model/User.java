package com.ziezix360.hm.patient.dao.model;

import com.ziezix360hm.DaoModel;

import javax.persistence.*;

public class User extends DaoModel {
    int id;
    String userName;
    int groupId;
    String name;
    String surname;
    String hashedSecret;

    public int getId() {
        return id;
    }

    public void setId(int id) {
        this.id = id;
    }

    public String getUserName() {
        return userName;
    }

    public void setUserName(String userName) {
        this.userName = userName;
    }

    public int getGroupId() {
        return groupId;
    }

    public void setGroupId(int groupId) {
        this.groupId = groupId;
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

    public String getHashedSecret() {
        return hashedSecret;
    }

    public void setHashedSecret(String secret) {
        this.hashedSecret = secret;
    }

}
