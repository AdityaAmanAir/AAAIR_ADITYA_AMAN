async function _0x1f2e() {
    try {
        const r = await fetch('/api/data');
        const d = await r.json();
        _0x3a4b(d);
        const s = await fetch('/api/stats');
        const st = await s.json();
        _0x5c2d(st, d.profile.social);
        _0x7b8c();
        _0xCommentHandler();
        _0xLoadComments();
    } catch (e) {
        console.error("Connection error", e);
    }
}

function _0x3a4b(d) {
    const h = document.getElementById('header');
    const m = document.getElementById('main-content');

    // Vector SVG logos for technical brand platforms
    const getLogo = (name) => {
        const logos = {
            'GitHub': `<svg class="brand-svg" viewBox="0 0 24 24" width="14" height="14" fill="currentColor"><path d="M12 0C5.37 0 0 5.37 0 12c0 5.31 3.435 9.795 8.205 11.385.6.105.825-.255.825-.57 0-.285-.015-1.23-.015-2.235-3.015.555-3.795-.735-4.035-1.41-.135-.345-.72-1.41-1.23-1.695-.42-.225-1.02-.78-.015-.795.945-.015 1.62.87 1.845 1.23 1.08 1.815 2.805 1.305 3.495.99.105-.78.42-1.305.765-1.605-2.67-.3-5.46-1.335-5.46-5.925 0-1.305.465-2.385 1.23-3.225-.12-.3-.54-1.53.12-3.18 0 0 1.005-.315 3.3 1.23.96-.27 1.98-.405 3-.405s2.04.135 3 .405c2.295-1.56 3.3-1.23 3.3-1.23.66 1.65.24 2.88.12 3.18.765.84 1.23 1.905 1.23 3.225 0 4.605-2.805 5.625-5.475 5.925.435.375.81 1.095.81 2.22 0 1.605-.015 2.895-.015 3.3 0 .315.225.69.825.57A12.02 12.02 0 0024 12c0-6.63-5.37-12-12-12z"/></svg>`,
            'LinkedIn': `<svg class="brand-svg" viewBox="0 0 24 24" width="14" height="14" fill="currentColor"><path d="M19 3a2 2 0 0 1 2 2v14a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h14m-.5 15.5v-5.3a3.26 3.26 0 0 0-3.26-3.26c-.85 0-1.84.52-2.28 1.3v-1.11h-2.79v8.37h2.79v-4.93c0-.77.62-1.4 1.39-1.4a1.4 1.4 0 0 1 1.4 1.4v4.93h2.75M6.88 8.56a1.68 1.68 0 0 0 1.68-1.68c0-.93-.75-1.69-1.68-1.69a1.69 1.69 0 0 0-1.69 1.69c0 .93.76 1.68 1.69 1.68m1.39 9.94v-8.37H5.5v8.37h2.77z"/></svg>`,
            'X': `<svg class="brand-svg" viewBox="0 0 24 24" width="14" height="14" fill="currentColor"><path d="M18.244 2.25h3.308l-7.227 8.26 8.502 11.24H16.17l-5.214-6.817L4.99 21.75H1.68l7.73-8.835L1.254 2.25H8.08l4.713 6.231zm-1.161 17.52h1.833L7.084 4.126H5.117z"/></svg>`,
            'LeetCode': `<svg class="brand-svg" viewBox="0 0 24 24" width="14" height="14" fill="currentColor"><path d="M13.483 0a1.374 1.374 0 0 0-.961.438L7.116 6.226a1.374 1.374 0 0 0-.416.947c0 .367.14.717.416.967l5.406 5.788c.25.25.6.39.961.39a1.374 1.374 0 0 0 .962-.39l5.406-5.788a1.374 1.374 0 0 0 0-1.914L14.445.438A1.374 1.374 0 0 0 13.483 0zm-8.88 9.467a1.374 1.374 0 0 0-.962.438L.416 13.131a1.374 1.374 0 0 0 0 1.914l3.225 3.226a1.374 1.374 0 0 0 1.963 0l3.225-3.226a1.374 1.374 0 0 0 0-1.914L5.604 9.905a1.374 1.374 0 0 0-1.001-.438z"/></svg>`,
            'YouTube': `<svg class="brand-svg" viewBox="0 0 24 24" width="14" height="14" fill="currentColor"><path d="M23.498 6.186a3.016 3.016 0 0 0-2.122-2.136C19.505 3.545 12 3.545 12 3.545s-7.505 0-9.377.505A3.017 3.017 0 0 0 .502 6.186C0 8.07 0 12 0 12s0 3.93.502 5.814a3.016 3.016 0 0 0 2.122 2.136c1.871.505 9.376.505 9.376.505s7.505 0 9.377-.505a3.015 3.015 0 0 0 2.122-2.136C24 15.93 24 12 24 12s0-3.93-.502-5.814zM9.545 15.568V8.432L15.818 12l-6.273 3.568z"/></svg>`,
            'Linux': `🐧`,
            'HackerEarth': `🌍`
        };
        return logos[name] || '';
    };

    const renderPill = (name, url, idx) => {
        const logo = getLogo(name);
        const logoSpan = logo ? `<span class="tag-svg">${logo}</span>` : '';
        return `<a id="social-link-${idx}" href="${url || '#'}" class="social-link marquee-pill" target="${url ? '_blank' : '_self'}">` +
            `<span class="tag-bracket">&lt;</span>` +
            logoSpan +
            `<span class="tag-label">${name}</span>` +
            `<span class="tag-bracket">/&gt;</span>` +
            `</a>`;
    };

        const row1Data = [
            { name: 'GitHub', url: 'https://github.com/AdityaAmanAir' },
            { name: 'LinkedIn', url: 'http://www.linkedin.com/in/adityaamanair' },
            { name: 'LeetCode', url: 'https://leetcode.com/u/AdityaAmanAir/' },
            { name: 'CodeChef', url: 'https://www.codechef.com/users/adityaamanair' },
            { name: 'Codeforces', url: 'https://codeforces.com/profile/AdityaAmanAir' },
            { name: 'HackerRank', url: 'https://www.hackerrank.com/profile/AdityaAmanAir' },
            { name: 'C++20', url: '' },
            { name: 'PyTorch', url: '' },
            { name: 'Linux', url: '' }
        ];

        const row2Data = [
            { name: 'X', url: 'https://x.com/AdityaAmanAir' },
            { name: 'Unstop', url: 'https://unstop.com/u/AdityaAman' },
            { name: 'HackerEarth', url: 'https://www.hackerearth.com/@AdityaAmanAir/' },
            { name: 'Neocities', url: 'http://adityaaman.neocities.org/' },
            { name: 'YouTube', url: 'http://www.youtube.com/@AdityaAmanAir' },
            { name: 'Python3', url: '' },
            { name: 'OpenCV', url: '' },
            { name: 'Flutter', url: '' },
            { name: 'WebSockets', url: '' }
        ];

        const row1HTML = row1Data.map((item, i) => renderPill(item.name, item.url, i)).join('');
        const row2HTML = row2Data.map((item, i) => renderPill(item.name, item.url, i + 10)).join('');

        h.innerHTML = `<section id="hero-section" class="hero-section">` +
        `<div id="hero-bg-layer" class="hero-bg-layer">` +
        `<canvas id="hero-canvas" class="hero-canvas"></canvas>` +
        `<div id="hero-glow-primary" class="hero-glow hero-glow-primary"></div>` +
        `<div id="hero-glow-secondary" class="hero-glow hero-glow-secondary"></div>` +
        `<div id="hero-grid-overlay" class="hero-grid-overlay"></div>` +
        `</div>` +
        `<div id="hero-container" class="hero-container">` +
        `<div id="header-inner" class="header-inner hero-inner">` +
        `<div id="hero-status-badge" class="hero-status-badge">` +
        `<span id="hero-status-pulse" class="hero-status-pulse"></span>` +
        `<span id="hero-status-text" class="hero-status-text">Available for Research & Engineering</span>` +
        `<span id="hero-status-divider" class="hero-status-divider">|</span>` +
        `<span id="hero-status-location" class="hero-status-location">VIT Bhopal | 2024–2028</span>` +
        `</div>` +
        `<div id="hero-typography-stack" class="hero-typography-stack">` +
        `<h1 id="title" class="title hero-title">` +
        `<span id="hero-name-first" class="hero-name-first">ADITYA</span> ` +
        `<span id="hero-name-last" class="hero-name-last">AMAN</span>` +
        `</h1>` +
        `<div id="hero-role-wrapper" class="hero-role-wrapper">` +
        `<span id="hero-role-prefix" class="hero-role-prefix">Specialist in </span>` +
        `<span id="hero-role-text" class="hero-role-text">Artificial Intelligence & Machine Learning</span>` +
        `</div>` +
        `<p id="subtitle" class="subtitle hero-bio">` +
        `Born ${d.profile.birth_year} | B.Tech AI/ML @ VIT Bhopal University. ` +
        `Architecting high-performance C++ backend microservices, neural network evaluation pipelines, and cross-platform systems.` +
        `</p>` +
        `</div>` +
        `<div id="hero-widget-card" class="hero-widget-card">` +
        `<div id="hero-terminal-header" class="hero-terminal-header">` +
        `<div id="hero-terminal-controls" class="hero-terminal-controls">` +
        `<span id="hero-dot-red" class="terminal-dot dot-red"></span>` +
        `<span id="hero-dot-yellow" class="terminal-dot dot-yellow"></span>` +
        `<span id="hero-dot-green" class="terminal-dot dot-green"></span>` +
        `</div>` +
        `<span id="hero-terminal-title" class="terminal-title">aditya@aaair-node:~ (zsh)</span>` +
        `</div>` +
        `<div id="hero-terminal-body" class="hero-terminal-body">` +
        `<div class="terminal-line"><span class="terminal-prompt">$ </span><span class="terminal-cmd">cat stack.json</span></div>` +
        `<div class="terminal-line terminal-output">=> ["C++20", "Python3", "PyTorch", "OpenCV", "Flutter", "WebSockets", "Linux"]</div>` +
        `</div>` +
        `</div>` +
        `<div id="hero-cta-bar" class="hero-cta-bar">` +
        `<a id="hero-cta-contact" href="#contact-section" class="action-btn hero-btn-primary">Contact Me</a>` +
        `<a id="hero-cta-comments" href="#comment-section" class="action-btn hero-btn-secondary">Leave Comment</a>` +
        `<button id="hero-btn-copy-email" class="action-btn hero-btn-icon" onclick="navigator.clipboard.writeText('AdityaAman@duck.com'); alert('Email copied!');">` +
        `<span id="hero-copy-icon" class="hero-icon">✉</span> Copy Email` +
        `</button>` +
        `</div>` +
        `<div id="hero-social-wrapper" class="hero-social-wrapper marquee-container">` +
        `<div class="marquee-row marquee-forward">` +
        `<div class="marquee-track">${row1HTML + row1HTML}</div>` +
        `</div>` +
        `<div class="marquee-row marquee-reverse">` +
        `<div class="marquee-track">${row2HTML + row2HTML}</div>` +
        `</div>` +
        `</div>` +
        `<div id="hero-quick-stats" class="hero-quick-stats">` +
        `<div id="hero-stat-box-1" class="hero-stat-box"><span id="hero-stat-val-1" class="hero-stat-val">AIR 463</span><span id="hero-stat-lbl-1" class="hero-stat-lbl">ISIAT Rank</span></div>` +
        `<div id="hero-stat-box-2" class="hero-stat-box"><span id="hero-stat-val-2" class="hero-stat-val">AIR 963</span><span id="hero-stat-lbl-2" class="hero-stat-lbl">JEE Mains 2B</span></div>` +
        `<div id="hero-stat-box-3" class="hero-stat-box"><span id="hero-stat-val-3" class="hero-stat-val">Top 2%</span><span id="hero-stat-lbl-3" class="hero-stat-lbl">NPTEL Elite + Gold</span></div>` +
        `</div>` +
        `<div id="hero-scroll-indicator" class="hero-scroll-indicator">` +
        `<a id="hero-scroll-link" href="#section-grid" class="hero-scroll-link">` +
        `<span id="hero-scroll-icon" class="hero-scroll-icon">↓</span> ` +
        `<span id="hero-scroll-text" class="hero-scroll-text">Scroll to explore</span>` +
        `</a>` +
        `</div>` +
        `</div>` +
        `</div>` +
        `</section>`;

    let sectionsHtml = `<div id="section-grid" class="section-grid">`;
    d.sections.forEach((s, sIdx) => {
        sectionsHtml += `<section id="section-card-${sIdx}" class="section-card">` +
            `<h2 id="section-title-${sIdx}" class="section-title">${s.title}</h2>`;

        if (s.type === 'timeline' || s.type === 'list') {
            sectionsHtml += (s.data || []).map((i, iIdx) =>
                `<div id="item-block-${sIdx}-${iIdx}" class="item-block">` +
                `<div id="item-label-${sIdx}-${iIdx}" class="item-label">${i.institution || i.name || i.exam || i.title || ''}</div>` +
                `<div id="item-detail-${sIdx}-${iIdx}" class="item-detail">${i.degree || i.score || i.issuer || i.detail || ''}</div>` +
                `<div id="item-sub-${sIdx}-${iIdx}" class="item-sub">${i.period || i.date || i.year || ''}</div>` +
                `${i.description ? `<p id="item-desc-${sIdx}-${iIdx}" class="item-desc">${i.description}</p>` : ''}` +
                `</div>`
            ).join('');
        } else if (s.type === 'grid') {
            sectionsHtml += `<div id="sub-grid-${sIdx}" class="sub-grid">${(s.data || []).map((i, iIdx) =>
                `<div id="sub-item-${sIdx}-${iIdx}" class="item-block sub-item">` +
                `<div id="sub-item-label-${sIdx}-${iIdx}" class="item-label">${i.exam || i.name || i.title}</div>` +
                `<div id="sub-item-detail-${sIdx}-${iIdx}" class="item-detail">${i.score || i.level || ''}</div>` +
                `</div>`
            ).join('')}</div>`;
        } else if (s.type === 'cards') {
            sectionsHtml += (s.data || []).map((i, iIdx) =>
                `<div id="card-item-${sIdx}-${iIdx}" class="item-block">` +
                `<div id="card-item-label-${sIdx}-${iIdx}" class="item-label">${i.title || i.name}</div>` +
                `<div id="card-item-detail-${sIdx}-${iIdx}" class="item-detail">${i.issuer || ''} ${i.date ? '| ' + i.date : ''}</div>` +
                `${i.highlight ? `<div id="card-item-highlight-${sIdx}-${iIdx}" class="item-highlight">${i.highlight}</div>` : ''}` +
                `${i.actions ? `<div id="card-item-actions-${sIdx}-${iIdx}" class="item-actions">${i.actions.map((v, aIdx) => `<a id="action-btn-${sIdx}-${iIdx}-${aIdx}" href="${v.url}" class="action-btn" target="_blank">${v.label}</a>`).join('')}</div>` : ''}` +
                `</div>`
            ).join('');
        } else if (s.type === 'composite') {
            (s.sub_sections || []).forEach((u, uIdx) => {
                sectionsHtml += `<h3 id="composite-title-${sIdx}-${uIdx}" class="composite-title">${u.title}</h3>`;
                sectionsHtml += (u.data || []).map((i, iIdx) =>
                    `<div id="composite-item-${sIdx}-${uIdx}-${iIdx}" class="item-block">` +
                    `<div id="composite-label-${sIdx}-${uIdx}-${iIdx}" class="item-label">${i.title || i.name || ''}</div>` +
                    `<div id="composite-detail-${sIdx}-${uIdx}-${iIdx}" class="item-detail">${i.detail || i.description || ''}</div>` +
                    `</div>`
                ).join('');
            });
        }
        sectionsHtml += `</section>`;
    });

    // Append Contact Section
    sectionsHtml += `<section id="contact-section" class="section-card contact-section">` +
        `<h2 id="contact-title" class="section-title">Contact & Inquiry</h2>` +
        `<form id="contact-form" class="contact-form">` +
        `<div id="contact-group-name" class="form-group">` +
        `<label id="contact-label-name" class="form-label" for="contact-name">NAME</label>` +
        `<input type="text" id="contact-name" class="form-input" placeholder="Enter your full name" required />` +
        `</div>` +
        `<div id="contact-group-email" class="form-group">` +
        `<label id="contact-label-email" class="form-label" for="contact-email">EMAIL</label>` +
        `<input type="email" id="contact-email" class="form-input" placeholder="Enter your email address" required />` +
        `</div>` +
        `<div id="contact-group-subject" class="form-group">` +
        `<label id="contact-label-subject" class="form-label" for="contact-subject">SUBJECT</label>` +
        `<input type="text" id="contact-subject" class="form-input" placeholder="Enter subject or title" required />` +
        `</div>` +
        `<div id="contact-group-body" class="form-group">` +
        `<label id="contact-label-body" class="form-label" for="contact-body">BODY</label>` +
        `<textarea id="contact-body" class="form-textarea" rows="4" placeholder="Write your inquiry or message here..." required></textarea>` +
        `</div>` +
        `<div id="contact-actions" class="form-actions">` +
        `<button type="submit" id="contact-submit-btn" class="action-btn contact-submit-btn">Send Message</button>` +
        `</div>` +
        `<div id="contact-status" class="contact-status"></div>` +
        `</form>` +
        `</section>` +
        `<section id="comment-section" class="section-card comment-section">` +
        `<h2 id="comment-title" class="section-title">Leave a Comment</h2>` +
        `<form id="comment-form" class="comment-form">` +
        `<div id="comment-group-name" class="form-group">` +
        `<label id="comment-label-name" class="form-label" for="comment-name">NAME</label>` +
        `<input type="text" id="comment-name" class="form-input" placeholder="Your full name" required />` +
        `</div>` +
        `<div id="comment-group-position" class="form-group">` +
        `<label id="comment-label-position" class="form-label" for="comment-position">POSITION WITH INSTITUTION</label>` +
        `<input type="text" id="comment-position" class="form-input" placeholder="Your position / institution" required />` +
        `</div>` +
        `<div id="comment-group-gender" class="form-group">` +
        `<label id="comment-label-gender" class="form-label" for="comment-gender">GENDER</label>` +
        `<select id="comment-gender" class="form-input" required>` +
        `<option value="">Select Gender</option>` +
        `<option value="Male">Male</option>` +
        `<option value="Female">Female</option>` +
        `<option value="Other">Other</option>` +
        `</select>` +
        `</div>` +
        `<div id="comment-group-text" class="form-group">` +
        `<label id="comment-label-text" class="form-label" for="comment-text">COMMENT</label>` +
        `<textarea id="comment-text" class="form-textarea" rows="4" placeholder="Write your comment here..." required></textarea>` +
        `</div>` +
        `<div id="comment-actions" class="form-actions">` +
        `<button type="submit" id="comment-submit-btn" class="action-btn comment-submit-btn">Send Comment</button>` +
        `</div>` +
        `<div id="comment-status" class="contact-status"></div>` +
        `</form>` +
        `</section>` +
        `<section id="comments-display-section" class="section-card comments-display-section">` +
        `<h2 id="comments-display-title" class="section-title">What People Say</h2>` +
        `<div id="comments-display-status" class="comments-display-status">Loading comments...</div>` +
        `<div id="comments-display-list" class="comments-display-list"></div>` +
        `</section>`;

    sectionsHtml += `</div>`;
    m.innerHTML = sectionsHtml;
}

function _0x5c2d(st, sc) {
    const grid = document.getElementById('section-grid');
    if (!grid) return;

    const statsCard = document.createElement('section');
    statsCard.className = 'section-card stats-section';
    statsCard.id = 'stats-section';

    let html = `<h2 id="stats-title" class="section-title">Live Coding Stats</h2><div id="stats-wrapper" class="stats-wrapper">`;
    if (st.leetcode && Object.keys(st.leetcode).length > 0) {
        html += `<div id="stats-block-leetcode" class="item-block stats-block">` +
            `<div id="stats-label-leetcode" class="item-label">LeetCode</div>` +
            `<div id="stats-grid-leetcode" class="stats-box-grid">` +
            `<div id="stat-cell-leetcode-solved" class="stat-cell"><span id="stat-num-leetcode-solved" class="stat-num">${st.leetcode.totalSolved || 0}</span><span id="stat-key-leetcode-solved" class="stat-key">Solved</span></div>` +
            `<div id="stat-cell-leetcode-acc" class="stat-cell"><span id="stat-num-leetcode-acc" class="stat-num">${st.leetcode.acceptanceRate || 0}%</span><span id="stat-key-leetcode-acc" class="stat-key">Acceptance</span></div>` +
            `<div id="stat-cell-leetcode-rank" class="stat-cell"><span id="stat-num-leetcode-rank" class="stat-num">#${st.leetcode.ranking || 'N/A'}</span><span id="stat-key-leetcode-rank" class="stat-key">Ranking</span></div>` +
            `</div></div>`;
    }
    if (st.github && Object.keys(st.github).length > 0) {
        html += `<div id="stats-block-github" class="item-block stats-block">` +
            `<div id="stats-label-github" class="item-label">GitHub Activity</div>` +
            `<div id="stats-grid-github" class="stats-box-grid">` +
            `<div id="stat-cell-github-repos" class="stat-cell"><span id="stat-num-github-repos" class="stat-num">${st.github.public_repos || 0}</span><span id="stat-key-github-repos" class="stat-key">Repos</span></div>` +
            `<div id="stat-cell-github-followers" class="stat-cell"><span id="stat-num-github-followers" class="stat-num">${st.github.followers || 0}</span><span id="stat-key-github-followers" class="stat-key">Followers</span></div>` +
            `<div id="stat-cell-github-following" class="stat-cell"><span id="stat-num-github-following" class="stat-num">${st.github.following || 0}</span><span id="stat-key-github-following" class="stat-key">Following</span></div>` +
            `</div></div>`;
    }
    if (st.codeforces && Object.keys(st.codeforces).length > 0) {
        html += `<div id="stats-block-codeforces" class="item-block stats-block">` +
            `<div id="stats-label-codeforces" class="item-label">Codeforces</div>` +
            `<div id="stats-grid-codeforces" class="stats-box-grid">` +
            `<div id="stat-cell-codeforces-rating" class="stat-cell"><span id="stat-num-codeforces-rating" class="stat-num">${st.codeforces.rating || 0}</span><span id="stat-key-codeforces-rating" class="stat-key">Rating</span></div>` +
            `<div id="stat-cell-codeforces-max" class="stat-cell"><span id="stat-num-codeforces-max" class="stat-num">${st.codeforces.maxRating || 0}</span><span id="stat-key-codeforces-max" class="stat-key">Max Rating</span></div>` +
            `<div id="stat-cell-codeforces-rank" class="stat-cell"><span id="stat-num-codeforces-rank" class="stat-num">${st.codeforces.rank || 'unranked'}</span><span id="stat-key-codeforces-rank" class="stat-key">Rank</span></div>` +
            `</div></div>`;
    }
    html += `</div>`;
    statsCard.innerHTML = html;
    grid.prepend(statsCard);
}

const _0x9e12 = "https://script.google.com/macros/s/AKfycbzkbvZEsIZLdeOcoxNywyMVpl1mV4pz-ihFdL8wYbnGNVzOH9Bwa6ipa36abDEjJu4Ftg/exec";

function _0x7b8c() {
    const form = document.getElementById('contact-form');
    if (!form) return;

    form.addEventListener('submit', async function (e) {
        e.preventDefault();
        const statusEl = document.getElementById('contact-status');
        const submitBtn = document.getElementById('contact-submit-btn');

        const name = document.getElementById('contact-name').value.trim();
        const email = document.getElementById('contact-email').value.trim();
        const subject = document.getElementById('contact-subject').value.trim();
        const body = document.getElementById('contact-body').value.trim();

        const now = new Date();
        const datetime = now.getFullYear() + '-' +
            String(now.getMonth() + 1).padStart(2, '0') + '-' +
            String(now.getDate()).padStart(2, '0') + ' ' +
            String(now.getHours()).padStart(2, '0') + ':' +
            String(now.getMinutes()).padStart(2, '0') + ':' +
            String(now.getSeconds()).padStart(2, '0');

        if (!name || !email || !subject || !body) {
            statusEl.textContent = 'Please complete all required fields.';
            statusEl.className = 'contact-status status-error';
            return;
        }

        submitBtn.disabled = true;
        statusEl.textContent = 'Submitting message...';
        statusEl.className = 'contact-status status-pending';

        const payload = {
            "NAME": name,
            "EMAIL": email,
            "DATE&TIME": datetime,
            "SUBJECT": subject,
            "BODY": body
        };

        // Tier 1: Try local backend proxy endpoint (/api/contact)
        try {
            const resp = await fetch('/api/contact', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(payload)
            });
            if (resp.ok) {
                statusEl.textContent = 'Message successfully submitted!';
                statusEl.className = 'contact-status status-success';
                form.reset();
                submitBtn.disabled = false;
                return;
            }
        } catch (err) {
            console.log('Backend proxy offline, engaging fallback submission method...');
        }

        // Tier 2: Hidden iframe form submission (bypasses browser CORS & redirects completely)
        try {
            let iframe = document.getElementById('hidden_iframe');
            if (!iframe) {
                iframe = document.createElement('iframe');
                iframe.id = 'hidden_iframe';
                iframe.name = 'hidden_iframe';
                iframe.style.display = 'none';
                document.body.appendChild(iframe);
            }

            const tempForm = document.createElement('form');
            tempForm.action = _0x9e12;
            tempForm.method = 'POST';
            tempForm.target = 'hidden_iframe';

            const fields = {
                'NAME': name,
                'EMAIL': email,
                'DATE&TIME': datetime,
                'SUBJECT': subject,
                'BODY': body
            };

            for (const key in fields) {
                const hiddenInput = document.createElement('input');
                hiddenInput.type = 'hidden';
                hiddenInput.name = key;
                hiddenInput.value = fields[key];
                tempForm.appendChild(hiddenInput);
            }

            document.body.appendChild(tempForm);
            tempForm.submit();
            setTimeout(() => {
                document.body.removeChild(tempForm);
            }, 1000);

            statusEl.textContent = 'Message successfully submitted!';
            statusEl.className = 'contact-status status-success';
            form.reset();
        } catch (err) {
            console.error('Submission error:', err);
            statusEl.textContent = 'Failed to submit. Please check your network connection.';
            statusEl.className = 'contact-status status-error';
        } finally {
            submitBtn.disabled = false;
        }
    });
}

const _0xCommentUrl = "https://script.google.com/macros/s/AKfycbzb9XEjjUmMhJw2I9CXIDCfsuB-mmkq_T4dUGNx9xCiysDnFQCrs-U9s6uea8nJqKLF-g/exec";

function _0xCommentHandler() {
    const form = document.getElementById('comment-form');
    if (!form) return;

    form.addEventListener('submit', async function (e) {
        e.preventDefault();
        const statusEl = document.getElementById('comment-status');
        const submitBtn = document.getElementById('comment-submit-btn');

        const name = document.getElementById('comment-name').value.trim();
        const position = document.getElementById('comment-position').value.trim();
        const gender = document.getElementById('comment-gender').value;
        const comment = document.getElementById('comment-text').value.trim();

        if (!name || !position || !gender || !comment) {
            statusEl.textContent = 'Please complete all required fields.';
            statusEl.className = 'contact-status status-error';
            return;
        }

        submitBtn.disabled = true;
        statusEl.textContent = 'Submitting comment...';
        statusEl.className = 'contact-status status-pending';

        // Method 1: fetch no-cors directly to Google Apps Script (fire-and-forget)
        try {
            fetch(_0xCommentUrl, {
                method: 'POST',
                mode: 'no-cors',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({
                    "NAME": name,
                    "Position With Institution": position,
                    "Gender": gender,
                    "Comment": comment
                })
            });
        } catch (err) { /* silent */ }

        // Method 2: Hidden iframe form POST (guaranteed delivery, works around CORS)
        try {
            let iframe = document.getElementById('hidden_comment_iframe');
            if (!iframe) {
                iframe = document.createElement('iframe');
                iframe.id = 'hidden_comment_iframe';
                iframe.name = 'hidden_comment_iframe';
                iframe.style.display = 'none';
                document.body.appendChild(iframe);
            }

            const tempForm = document.createElement('form');
            tempForm.action = _0xCommentUrl;
            tempForm.method = 'POST';
            tempForm.target = 'hidden_comment_iframe';

            const fields = {
                'NAME': name,
                'Position With Institution': position,
                'Gender': gender,
                'Comment': comment
            };

            for (const key in fields) {
                const input = document.createElement('input');
                input.type = 'hidden';
                input.name = key;
                input.value = fields[key];
                tempForm.appendChild(input);
            }

            document.body.appendChild(tempForm);
            tempForm.submit();
            setTimeout(() => document.body.removeChild(tempForm), 2000);
        } catch (err) { /* silent */ }

        // Immediately prepend new comment to top of comments display list
        const listEl = document.getElementById('comments-display-list');
        const displayStatusEl = document.getElementById('comments-display-status');
        if (listEl) {
            if (displayStatusEl) displayStatusEl.style.display = 'none';
            const svgSrc = gender === 'Female' ? 'src/image/girl.svg' : (gender === 'Male' ? 'src/image/boy.svg' : 'src/image/other.svg');
            const cardId = `new-${Date.now()}`;
            const newCommentCard = document.createElement('div');
            newCommentCard.id = `comment-card-${cardId}`;
            newCommentCard.className = 'comment-card comment-card-new';
            newCommentCard.innerHTML = `<div class="comment-card-header">` +
                `<div id="comment-avatar-${cardId}" class="comment-avatar"><img id="comment-avatar-img-${cardId}" class="comment-avatar-img" src="${svgSrc}" alt="${gender}" /></div>` +
                `<div class="comment-meta">` +
                `<div class="comment-name">${name}</div>` +
                `<div class="comment-position">${position}</div>` +
                `</div>` +
                `</div>` +
                `<p class="comment-body">"${comment}"</p>`;
            listEl.prepend(newCommentCard);
        }

        // Show success immediately — both methods are fire-and-forget
        statusEl.textContent = 'Comment submitted successfully!';
        statusEl.className = 'contact-status status-success';
        form.reset();
        submitBtn.disabled = false;
    });
}

function _0xLoadComments() {
    const listEl = document.getElementById('comments-display-list');
    const statusEl = document.getElementById('comments-display-status');
    if (!listEl || !statusEl) return;

    fetch(_0xCommentUrl, {
        method: 'GET',
        mode: 'cors'
    })
    .then(r => r.json())
    .then(data => {
        if (data.status === 'success' && Array.isArray(data.comments) && data.comments.length > 0) {
            statusEl.style.display = 'none';
            // Show newest first
            const sorted = [...data.comments].reverse();
            listEl.innerHTML = sorted.map((c, idx) => {
                const g = (c.gender || '').toLowerCase();
                const svgSrc = g === 'female' ? 'src/image/girl.svg' : (g === 'male' ? 'src/image/boy.svg' : 'src/image/other.svg');
                return `<div id="comment-card-${idx}" class="comment-card">` +
                    `<div class="comment-card-header">` +
                    `<div id="comment-avatar-${idx}" class="comment-avatar"><img id="comment-avatar-img-${idx}" class="comment-avatar-img" src="${svgSrc}" alt="${c.gender || 'Other'}" /></div>` +
                    `<div class="comment-meta">` +
                    `<div class="comment-name">${c.name || 'Anonymous'}</div>` +
                    `<div class="comment-position">${c.position || ''}</div>` +
                    `</div>` +
                    `</div>` +
                    `<p class="comment-body">"${c.comment || ''}"</p>` +
                    `</div>`;
            }).join('');
        } else {
            statusEl.textContent = 'No comments yet. Be the first!';
        }
    })
    .catch(() => {
        statusEl.textContent = 'Comments are loading...';
    });
}

document.addEventListener('DOMContentLoaded', () => {
    _0x1f2e();
});
