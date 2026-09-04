(function () {
    'use strict';

    // ── Disable Right-Click ──
    document.addEventListener('contextmenu', e => e.preventDefault());

    // ── Disable Keyboard Shortcuts ──
    document.addEventListener('keydown', e => {
        if (
            e.keyCode === 123 ||
            (e.ctrlKey && e.shiftKey && (e.keyCode === 73 || e.keyCode === 74)) ||
            (e.ctrlKey && e.keyCode === 85) ||
            (e.ctrlKey && e.keyCode === 83)
        ) {
            e.preventDefault();
            return false;
        }
    });

    // ── Anti-Debugging Traps ──
    const trap = () => {
        try {
            const check = function () {
                if (typeof window.outerHeight !== 'undefined' && window.outerHeight - window.innerHeight > 160) {
                }
                debugger;
            };
            check();
        } catch (e) { }
    };

    setInterval(trap, 2000);

    // ── Content Protection ──
    window.addEventListener('load', () => {
        document.body.classList.add('no-select');
    });
})();
