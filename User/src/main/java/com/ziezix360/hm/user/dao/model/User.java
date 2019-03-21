package com.ziezix360.hm.user.dao.model;

import com.ziezix360hm.DaoModel;

import javax.persistence.*;

@Entity(name = "User")
@Table(name = "USERS",schema = "HM")
public class User extends DaoModel {
    @Column(name = "ID") @Id @GeneratedValue(strategy = GenerationType.IDENTITY)
    int id;
    @Column(name = "USER_NAME")
    String userName;
    @Column(name = "GROUP_ID")
    int groupId;
    @Column(name = "NAME")
    String name;
    @Column(name = "SURNAME")
    String surname;
    @Column(name = "HASHED_SECRET")
    String hashedSecret;

    public User() {
        super();
    }

    public User(String userName) {
        super();
        setUserName(userName);
    }

    public User(int id) {
        super();
        setId(id);
    }

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
