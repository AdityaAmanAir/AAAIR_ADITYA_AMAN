(function () {
    const isMobile = window.matchMedia("(max-width: 768px)").matches;
    if (isMobile) return;

    // States
    const IDLE = 0, WALK = 1, GROOM = 2, SLEEP = 3;
    let state = IDLE;
    let catX = 200;
    let catY = 400;
    let targetX = catX;
    let targetY = catY;
    let facing = 1; // 1 = right, -1 = left
    let tick = 0;
    let stateEnd = 0;
    let walkPhase = 0;

    // Wait for DOM
    function init() {
        const wrapper = document.getElementById('layout-wrapper');
        if (!wrapper) { setTimeout(init, 500); return; }

        // Container — absolute inside the page so it scrolls with content
        const el = document.createElement('div');
        el.id = 'neko-cat';
        el.setAttribute('aria-hidden', 'true');
        Object.assign(el.style, {
            position: 'absolute',
            width: '40px',
            height: '40px',
            zIndex: '50',
            pointerEvents: 'none',
            left: catX + 'px',
            top: catY + 'px'
        });
        wrapper.style.position = 'relative';
        wrapper.appendChild(el);

        // --- High-quality white pixel cat SVG ---
        function renderCat(legOffset, eyeRy, showZzz, showPaw, tailWag) {
            const tw = tailWag || 0;
            return `<svg viewBox="0 0 40 40" width="40" height="40" xmlns="http://www.w3.org/2000/svg" style="image-rendering:auto;">
                <!-- Tail -->
                <path d="M8 26 Q4 ${22 + tw} 3 ${18 + tw} Q2 ${14 + tw} 5 ${12 + tw}" stroke="#E8E8E8" stroke-width="3" stroke-linecap="round" fill="none"/>
                <!-- Body -->
                <ellipse cx="20" cy="25" rx="10" ry="7" fill="#F5F5F5"/>
                <ellipse cx="20" cy="26" rx="8" ry="5" fill="#FFFFFF"/>
                <!-- Back legs -->
                <rect x="11" y="${28 + legOffset[2]}" width="3.5" height="7" rx="1.8" fill="#E0E0E0"/>
                <rect x="25" y="${28 + legOffset[3]}" width="3.5" height="7" rx="1.8" fill="#E0E0E0"/>
                <!-- Paws back -->
                <ellipse cx="12.8" cy="${35.5 + legOffset[2]}" rx="2.2" ry="1.2" fill="#FFFFFF"/>
                <ellipse cx="26.8" cy="${35.5 + legOffset[3]}" rx="2.2" ry="1.2" fill="#FFFFFF"/>
                <!-- Front legs -->
                <rect x="14" y="${28 + legOffset[0]}" width="3.5" height="8" rx="1.8" fill="#F0F0F0"/>
                <rect x="22" y="${28 + legOffset[1]}" width="3.5" height="8" rx="1.8" fill="#F0F0F0"/>
                <!-- Paws front -->
                <ellipse cx="15.8" cy="${36.5 + legOffset[0]}" rx="2.2" ry="1.3" fill="#FFFFFF"/>
                <ellipse cx="23.8" cy="${36.5 + legOffset[1]}" rx="2.2" ry="1.3" fill="#FFFFFF"/>
                <!-- Head -->
                <circle cx="20" cy="14" r="9.5" fill="#F5F5F5"/>
                <!-- Inner ears -->
                <path d="M12 12 L8 2 L17 9 Z" fill="#F5F5F5"/>
                <path d="M28 12 L32 2 L23 9 Z" fill="#F5F5F5"/>
                <path d="M12.5 10 L9.5 4 L16 8.5 Z" fill="#F8C0D0"/>
                <path d="M27.5 10 L30.5 4 L24 8.5 Z" fill="#F8C0D0"/>
                <!-- Face -->
                <ellipse cx="20" cy="17" rx="4" ry="3" fill="#FFFFFF"/>
                <!-- Nose -->
                <ellipse cx="20" cy="15.8" rx="1.2" ry="0.9" fill="#F8A0B0"/>
                <!-- Mouth -->
                <path d="M18.5 17.5 Q20 19 21.5 17.5" stroke="#888" stroke-width="0.8" fill="none" stroke-linecap="round"/>
                <!-- Eyes -->
                <ellipse cx="15.5" cy="13" rx="2" ry="${eyeRy}" fill="#1A1A2E"/>
                <ellipse cx="24.5" cy="13" rx="2" ry="${eyeRy}" fill="#1A1A2E"/>
                ${eyeRy > 1 ? `
                    <circle cx="16.2" cy="12.2" r="0.8" fill="#FFFFFF"/>
                    <circle cx="25.2" cy="12.2" r="0.8" fill="#FFFFFF"/>
                ` : ''}
                <!-- Whiskers -->
                <line x1="10" y1="15" x2="3" y2="13.5" stroke="#CCC" stroke-width="0.6"/>
                <line x1="10" y1="16.5" x2="3" y2="17" stroke="#CCC" stroke-width="0.6"/>
                <line x1="30" y1="15" x2="37" y2="13.5" stroke="#CCC" stroke-width="0.6"/>
                <line x1="30" y1="16.5" x2="37" y2="17" stroke="#CCC" stroke-width="0.6"/>
                ${showPaw ? `<circle cx="28" cy="10" r="3" fill="#FFFFFF" stroke="#E0E0E0" stroke-width="0.5"/>` : ''}
                ${showZzz ? `
                    <text x="30" y="6" fill="#E5A93C" font-family="monospace" font-weight="700" font-size="7" opacity="0.7">z</text>
                    <text x="34" y="2" fill="#E5A93C" font-family="monospace" font-weight="700" font-size="5" opacity="0.5">z</text>
                ` : ''}
            </svg>`;
        }

        // Pick random point inside page content area
        function pickTarget() {
            const rect = wrapper.getBoundingClientRect();
            const pageW = wrapper.scrollWidth;
            const pageH = wrapper.scrollHeight;
            const margin = 60;
            targetX = margin + Math.random() * Math.max(100, pageW - margin * 2);
            targetY = margin + Math.random() * Math.min(pageH - margin, window.innerHeight + window.scrollY - margin);
        }

        // Schedule next behavior
        function nextState() {
            const r = Math.random();
            if (r < 0.40) {
                state = WALK;
                pickTarget();
                stateEnd = Date.now() + 5000 + Math.random() * 8000;
            } else if (r < 0.65) {
                state = IDLE;
                stateEnd = Date.now() + 2500 + Math.random() * 5000;
            } else if (r < 0.82) {
                state = GROOM;
                stateEnd = Date.now() + 3000 + Math.random() * 4000;
            } else {
                state = SLEEP;
                stateEnd = Date.now() + 5000 + Math.random() * 10000;
            }
        }

        nextState();

        // Main loop
        function frame() {
            tick++;
            const now = Date.now();

            // Transition check (except walk which transitions on arrival)
            if (state !== WALK && now > stateEnd) nextState();

            let legs = [0, 0, 0, 0];
            let eyeRy = 2.8;
            let showZzz = false;
            let showPaw = false;
            let tailWag = 0;

            if (state === WALK) {
                const dx = targetX - catX;
                const dy = targetY - catY;
                const dist = Math.hypot(dx, dy);

                if (dist < 8 || now > stateEnd) {
                    nextState();
                } else {
                    const angle = Math.atan2(dy, dx);
                    const speed = 1.2 + Math.random() * 0.3;
                    catX += Math.cos(angle) * speed;
                    catY += Math.sin(angle) * speed;
                    facing = dx >= 0 ? 1 : -1;

                    // Walking leg animation
                    walkPhase += 0.22;
                    const s = Math.sin(walkPhase);
                    const c = Math.cos(walkPhase);
                    legs = [s * 3, c * 3, -s * 2.5, -c * 2.5];
                    tailWag = Math.sin(walkPhase * 0.7) * 4;
                }
            } else if (state === IDLE) {
                // Gentle tail sway, occasional blink
                tailWag = Math.sin(tick * 0.06) * 2.5;
                if (tick % 150 < 6) eyeRy = 0.4;
            } else if (state === GROOM) {
                showPaw = true;
                tailWag = Math.sin(tick * 0.04) * 1.5;
                if (tick % 20 < 10) eyeRy = 1.0;
            } else if (state === SLEEP) {
                eyeRy = 0.3;
                showZzz = true;
                tailWag = 0;
            }

            // Render
            el.innerHTML = renderCat(legs, eyeRy, showZzz, showPaw, tailWag);
            el.style.left = (catX - 20) + 'px';
            el.style.top = (catY - 20) + 'px';
            el.style.transform = `scaleX(${facing})`;

            requestAnimationFrame(frame);
        }

        requestAnimationFrame(frame);
    }

    // Boot after page loads
    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', init);
    } else {
        init();
    }
})();
