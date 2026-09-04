#include <portfolio/service/PortfolioService.hpp>

namespace portfolio {
namespace service {

using json = nlohmann::json;

json PortfolioService::getProfile() const {
    return {
        {"name", "Aditya Aman"},
        {"specialization", "B.Tech Computer Science - Artificial Intelligence and Machine Learning"},
        {"university", "VIT Bhopal University"},
        {"bio", "A passionate AI/ML student at VIT Bhopal University with a strong foundation in problem-solving and software engineering."},
        {"skills", {"Problem Solving", "Python", "Machine Learning", "Calculus", "Statistics", "Artificial Intelligence", "Data Structures and Algorithms", "C++"}}
    };
}

json PortfolioService::getEducation() const {
    return {
        {
            {"institution", "VIT Bhopal University (VIT), Bhopal"},
            {"degree", "B. Tech/BE - Computer Science & Engineering [Artificial Intelligence (AI) & Machine Learning (ML)]"},
            {"period", "2024 - Present Full-time"},
            {"roll", "10129"},
            {"skills", {"MatLab", "Machine Learning", "Artificial Intelligence", "Electrical Circuits", "Calculus", "Communication"}}
        },
        {
            {"institution", "St. Xavier's School, Doranda, Ranchi"},
            {"degree", "Intermediate - Science"},
            {"period", "2023 - 2024 Full-time"},
            {"roll", "131"}
        },
        {
            {"institution", "Bishop's School, Ranchi"},
            {"degree", "High school - Science (ICSE)"},
            {"period", "2021 - 2022 Full-time"},
            {"roll", "123"}
        }
    };
}

json PortfolioService::getCertificates() const {
    return {
        {"Statistical Learning", "United Latino Students Association (Nov 2025)"},
        {"Programming with Generative AI (IISc Bangalore)", "NPTEL (Nov 2025)"},
        {"MATLAB Onramp", "MathWorks (Aug 2024)"},
        {"Python Essentials", "Vityarthi (Apr 2025)"},
        {"Fundamentals of AI & ML", "Vityarthi (Sep 2024)"},
        {"Introduction to Javascript and Ajax: Building Web Apps", "Johns Hopkins University (Feb 2026)"},
        {"Introduction to CSS3", "Johns Hopkins University (Jan 2026)"},
        {"Coding the Static Restaurant Site", "Johns Hopkins University (Jan 2026)"},
        {"HTML CSS and JavaScript for Web Developers", "Johns Hopkins University (Dec 2025)"},
        {"Indian School Certificate", "CISCE"},
        {"CHITRA BHUSHAN PART-2", "Fine Art (2nd Year)"},
        {"SIP Abacus Grand Master Level A", "SIP Abacus"},
        {"Pythagoras Pro", "Pythagoras"}
    };
}

json PortfolioService::getAwards() const {
    return {
        {"Abacus Advance Level 4", "Demonstrates exceptional proficiency in advanced abacus."},
        {"Rising star (Abacus)", "Qualifying proficiency in mental arithmetic, problem-solving, and concentration."},
        {"Rising Star (Fine Art)", "Awarded for 4th position and exceptional artistic talent."},
        {"SOF- IMO School Lv. Bronze", "Olympiad conducted by SOF in 2018-19. Bronze at school."},
        {"ADVANCE LEVEL 1 SIP", "Another level of SIP abacus after foundation. 2nd in batch."},
        {"Advance Level 2 SIP", "Enhances triple digits multiplication with double digits."},
        {"SIP Abacus Grand Master Level A", "At Ranchi, Jharkhand. Grand Master A level."}
    };
}

} // namespace service
} // namespace portfolio
