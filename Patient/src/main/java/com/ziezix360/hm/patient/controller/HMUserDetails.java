package com.ziezix360.hm.patient.controller;

import com.ziezix360.hm.patient.dao.model.User;
import org.springframework.security.core.GrantedAuthority;
import org.springframework.security.core.userdetails.UserDetails;

import java.util.Collection;

public class HMUserDetails implements UserDetails {

    private User user;

    HMUserDetails(User user) {
        this.user = user;
        this.user.setUserName(user.getUserName().trim());
        this.user.setHashedSecret(user.getHashedSecret().trim());
    }

    @Override
    public Collection<? extends GrantedAuthority> getAuthorities() {
        return null;
    }

    @Override
    public String getPassword() {
        return user.getHashedSecret();
    }

    @Override
    public String getUsername() {
        return user.getUserName();
    }

    @Override
    public boolean isAccountNonExpired() {
        return true;
    }

    @Override
    public boolean isAccountNonLocked() {
        return true;
    }

    @Override
    public boolean isCredentialsNonExpired() {
        return true;
    }

    @Override
    public boolean isEnabled() {
        return true;
    }
}
