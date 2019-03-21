package com.ziezix360.hm.patient.controller;

import com.ziezix360.hm.patient.dao.model.User;
import com.ziezix360.hm.patient.dao.model.UserRequest;
import com.ziezix360.hm.patient.dao.model.UserResponse;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.http.MediaType;
import org.springframework.security.core.userdetails.UserDetails;
import org.springframework.security.core.userdetails.UserDetailsService;
import org.springframework.security.core.userdetails.UsernameNotFoundException;
import org.springframework.security.crypto.bcrypt.BCryptPasswordEncoder;
import org.springframework.stereotype.Component;
import org.springframework.web.reactive.function.client.WebClient;

import java.time.Duration;

@Component
public class HMUserDetailsService implements UserDetailsService {

    @Value("${webclienturls.user}")
    String userUrl = "";

    BCryptPasswordEncoder passwordEncoder = new BCryptPasswordEncoder();

    @Override
    public UserDetails loadUserByUsername(String username) throws UsernameNotFoundException {

        UserRequest request = new UserRequest();
        request.setMethod("details");
        request.setUser(new User());
        request.getUser().setUserName(username);

        WebClient userClient = WebClient.create(userUrl);
        UserResponse response = userClient.post()
                .uri("details")
                .contentType(MediaType.APPLICATION_JSON)
                .syncBody(request)
                .retrieve()
                .bodyToMono(UserResponse.class)
                .block(Duration.ofMillis(2500));

        if (response != null && response.getUser() != null) {
            response.getUser().setHashedSecret("{bcrypt}" +
                    passwordEncoder.encode(response.getUser().getHashedSecret().trim()));
            return new HMUserDetails(response.getUser());
        }

        return null;
    }


}
