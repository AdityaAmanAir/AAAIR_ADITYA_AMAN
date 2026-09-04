# Project Architecture & Developer Guide

This document serves as the master architectural reference for human developers and AI coding agents working on this project.

---

## 1. System Overview

This application is a **high-performance, zero-clutter C++ microservice** powering a dynamic Single Page Application (SPA).

- **Backend**: Lightweight, multithreaded C++17 server built on `cpp-httplib`, compiled directly with `g++` (`O3` optimized).
- **Frontend**: Theme-agnostic Vanilla JavaScript (SPA) paired with decoupled, structure-first CSS.
- **Networking**: Dual-stack IPv4/IPv6 network listener bound to `0.0.0.0` (accessible across local Wi-Fi / LAN).
- **Caching**: Developer-friendly `no-store` cache headers for instant frontend visual feedback on edits.

---

## 2. Core Architectural Rules (STRICT CONSTRAINTS)

When modifying or expanding this repository, **all AI assistants and human developers MUST adhere to the following rules**:

### Rule 1: Zero Hardcoded Styling in JavaScript or HTML
- **HTML (`index.html`)** and **JavaScript (`app.js`)** MUST ONLY emit semantic HTML structure, CSS class names, and element-level IDs.
- **NEVER** write inline `style="..."` attributes or hardcoded color/layout values in DOM-generating JavaScript functions.

### Rule 2: 100% Presentational & Positional Independence in CSS
- **Visual Presentation**: Theme colors, fonts, glassmorphism, borders, hover states, and animations MUST be defined exclusively in CSS files (`style.css`, `styleSTRUCTURE.css`, etc.).
- **Positional Ordering**: Section order, component flow, header item positioning, and grid layouts MUST be controllable purely via CSS using CSS Grid Areas (`grid-template-areas`) or Flexbox ordering (`order: N;`).

### Rule 3: Dynamic Data API Integration
- Page content is served via `/api/data` (populated from `data.json`).
- Live coding statistics (LeetCode, GitHub, Codeforces) are served via `/api/stats` proxied securely by the C++ backend.

---

## 3. Directory & File Blueprint

```
.
├── backend/
│   ├── include/          # C++ header files & SecurityMiddleware
│   ├── src/              # Server execution logic & API controllers
│   └── third_party/      # Embedded single-header libraries (nlohmann/json, httplib)
├── frontend/
│   ├── css/
│   │   ├── style.css             # Active stylesheet loaded by index.html
│   │   ├── styleSTRUCTURE.css    # Master developer blueprint (complete DOM selector skeleton)
│   │   └── styleTEST1.css        # Theme example demonstrating section reordering via CSS
│   ├── js/
│   │   ├── app.js                # Dynamic SPA DOM builder & data fetcher
│   │   └── security.js           # Client-side anti-tamper / inspection guard
│   └── index.html                # Semantic root HTML skeleton
├── data.json                     # Primary JSON dataset (Profile, Sections, Content)
├── CMakeLists.txt                # CMake build configuration
├── README.md                     # Setup, build, and run instructions
└── ARCHITECTURE.md               # Developer & AI reference guide (This document)
```

---

## 4. Selector & DOM Hierarchy Blueprint

Every element generated in the DOM carries explicit semantic IDs and classes for CSS targeting:

| DOM Element | Selector | Purpose |
| :--- | :--- | :--- |
| `<body>` | `#body`, `.page-body` | Global font, background, and page container |
| `<div>` | `#bg-layer`, `.bg-layer` | Fixed background layer for canvas/radial ambient glows |
| `<div>` | `#layout-wrapper`, `.layout-wrapper` | Main width container & page grid wrapper |
| `<header>` | `#header`, `.header-container` | Top header container |
| `<div>` | `.header-inner` | Flex column box holding title, bio, and social nav |
| `<h1>` | `#title`, `.title` | Owner / site title |
| `<p>` | `#subtitle`, `.subtitle` | Bio tagline / secondary title |
| `<nav>` | `#social-nav`, `.social-nav` | Social navigation wrapper |
| `<a>` | `.social-link` | Social channel pill buttons |
| `<main>` | `#main-content`, `.content-container` | Primary section grid wrapper |
| `<div>` | `#section-grid`, `.section-grid` | CSS Grid container holding cards |
| `<section>` | `.section-card` / `#section-card-${N}` | Individual content section cards |
| `<h2>` | `.section-title` | Section title heading |
| `<div>` | `.item-block` / `#item-block-${S}-${I}` | Entry block container |
| `<section>` | `#stats-section`, `.stats-section` | Live coding stats card banner |
| `<div>` | `.stats-box-grid` | Grid holding metric cells |
| `<div>` | `.stat-cell` | Metric card (number + label descriptor) |

---

## 5. How to Perform Common Tasks

### Task A: How to Change Component Layout & Section Order via CSS
To reorder sections on the page (e.g. moving Education to the bottom) without modifying JS or HTML:

```css
/* 1. Make the section grid a flex column */
#section-grid {
    display: flex;
    flex-direction: column;
    gap: 1.5rem;
}

/* 2. Set default order for all section cards */
.section-card {
    order: 1;
}

/* 3. Reorder specific section using its index ID */
#section-card-0 {
    order: 999 !important; /* Pushes Education section to the bottom */
}
```

### Task B: How to Swap Themes
1. Create a new stylesheet inside `frontend/css/` (e.g. `styleDARK.css`).
2. Copy the template selectors from `frontend/css/styleSTRUCTURE.css`.
3. Fill in your visual styles.
4. Overwrite `frontend/css/style.css` with your new stylesheet. Changes will reflect instantly without browser caching issues.

### Task C: How to Build & Run the Backend
```bash
# 1. Configure the CMake build
cmake -S . -B build

# 2. Compile the C++ microservice
cmake --build build

# 3. Start the server
./build/server
```
The server will listen on `0.0.0.0:8080` (accessible via `http://localhost:8080` or your machine's local Wi-Fi IP `http://<YOUR_LOCAL_IP>:8080`).

---

## 6. Recommendations for Future Development

- **Adding New Sections**: Add entries to `data.json` under `sections`. `app.js` will automatically render them with semantic IDs (`#section-card-${index}`).
- **Adding Custom Styling Elements**: Use CSS pseudo-elements (`::before`, `::after`) inside theme stylesheets to create divider lines, accent borders, or bullet icons without cluttering the DOM.
- **HTTPS Enablement**: If deploying over public networks, update `backend/src/main.cpp` to use `httplib::SSLServer`.
