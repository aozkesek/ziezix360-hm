package com.ziezix360.hm.patient.controller;

import com.ziezix360.hm.patient.dao.intf.PatientDao;
import com.ziezix360.hm.patient.dao.model.Patient;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.validation.BindingResult;
import org.springframework.web.bind.annotation.*;


@Controller
public class PatientIndex {

    @Autowired
    PatientDao patientDao;

    @RequestMapping("/signin")
    public String signIn() {
        return "/signin";
    }

    @RequestMapping("/signin-error")
    public String signInError() {
        return "/signin-error";
    }

    @RequestMapping("/signedin")
    public String signedIn() {
        return "redirect:/patients";
    }

    @RequestMapping("/loggedout")
    public String loggedOut() {
        return "/loggedout";
    }

    @RequestMapping({"/patients", "/"})
    public String patientHome(Model model) {
        model.addAttribute("addOrUpdate", "");
        model.addAttribute("patients", patientDao.list());
        return "/patients";
    }

    @RequestMapping("/patients/patient/{id}")
    String patientRead(@PathVariable int id, Model model) {
        Patient patient = new Patient();
        patient.setId(id);
        patient = patientDao.read(patient);
        model.addAttribute("patient", patient);
        model.addAttribute("addOrUpdate", "update");
        return "/patient";
    }

    @RequestMapping("/patients/patient/{id}/bio")
    String patientBio(@PathVariable int id, Model model) {
        Patient patient = new Patient();
        patient.setId(id);
        patient = patientDao.read(patient);
        model.addAttribute("patient", patient);
        return "/patientbio";
    }

    @RequestMapping("/patients/new")
    String patientNew(Model model) {
        model.addAttribute("addOrUpdate", "add");
        model.addAttribute("patient", new Patient());
        return "/patient";
    }

    @PostMapping("/patients/update")
    String patientUpdate(Patient patient, BindingResult bindingResult, Model model) {
        patient = patientDao.update(patient);
        model.addAttribute("patient", null);
        model.addAttribute("patients", patientDao.list());
        return "redirect:/patients";
    }

    @PostMapping("/patients/add")
    String patientAdd(Patient patient, BindingResult bindingResult, Model model) {
        patientDao.create(patient);
        model.addAttribute("patients", patientDao.list());
        return "redirect:/patients";
    }

}
