import * as THREE from 'three';
import { GLTFLoader } from 'three/addons/loaders/GLTFLoader.js';
import { OrbitControls } from 'three/addons/controls/OrbitControls.js';

const initializedViewers = new WeakSet();

function initializeViewer(container) {
    if (initializedViewers.has(container)) return;
    initializedViewers.add(container);

    const status = container.querySelector('.model-viewer-status');
    const title = container.dataset.modelTitle || '3D model';

    try {
        const scene = new THREE.Scene();
        const camera = new THREE.PerspectiveCamera(35, 1, 0.1, 100);
        camera.position.set(0, 0, 4);

        const renderer = new THREE.WebGLRenderer({ antialias: true, alpha: true });
        renderer.setPixelRatio(Math.min(window.devicePixelRatio || 1, 2));
        renderer.outputColorSpace = THREE.SRGBColorSpace;
        renderer.setSize(container.clientWidth, container.clientHeight);
        renderer.domElement.setAttribute('aria-label', `${title}. Drag to rotate.`);
        container.prepend(renderer.domElement);

        scene.add(new THREE.HemisphereLight(0xffffff, 0x50596a, 2.2));
        const keyLight = new THREE.DirectionalLight(0xffffff, 2.4);
        keyLight.position.set(3, 5, 4);
        scene.add(keyLight);

        const controls = new OrbitControls(camera, renderer.domElement);
        controls.enableDamping = true;
        controls.dampingFactor = 0.06;
        controls.enablePan = false;
        controls.enableZoom = false;
        controls.autoRotate = true;
        controls.autoRotateSpeed = 0.7;

        const resizeObserver = new ResizeObserver(() => {
            const width = container.clientWidth;
            const height = container.clientHeight;
            if (width === 0 || height === 0) return;
            renderer.setSize(width, height, false);
            camera.aspect = width / height;
            camera.updateProjectionMatrix();
        });
        resizeObserver.observe(container);

        new GLTFLoader().load(
            container.dataset.modelUrl,
            (gltf) => {
                const model = gltf.scene;
                const bounds = new THREE.Box3().setFromObject(model);
                const size = bounds.getSize(new THREE.Vector3());
                const center = bounds.getCenter(new THREE.Vector3());
                const largestSide = Math.max(size.x, size.y, size.z);

                model.position.sub(center);
                if (largestSide > 0) model.scale.setScalar(2.4 / largestSide);
                scene.add(model);
                controls.update();
                if (status) status.remove();
            },
            undefined,
            () => {
                if (status) status.textContent = 'Unable to load this 3D model.';
            }
        );

        function render() {
            requestAnimationFrame(render);
            controls.update();
            renderer.render(scene, camera);
        }
        render();
    } catch (error) {
        if (status) status.textContent = '3D preview is not available in this browser.';
    }
}

function initializeExistingViewers() {
    document.querySelectorAll('.model-viewer[data-model-url]').forEach(initializeViewer);
}

new MutationObserver(initializeExistingViewers).observe(document.body, {
    childList: true,
    subtree: true
});
initializeExistingViewers();