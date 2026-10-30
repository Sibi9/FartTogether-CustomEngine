// CustomEngine Client Simulator & Interactive Controller
document.addEventListener('DOMContentLoaded', () => {

    // 1. إدارة أزرار النافذة غير المؤطرة (Frameless Window Controls)
    const windowFrame = document.getElementById('windowFrame');
    const btnMinimize = document.getElementById('btnMinimize');
    const btnMaximize = document.getElementById('btnMaximize');
    const btnClose = document.getElementById('btnClose');
    const titleBar = document.getElementById('titleBar');

    let isMaximized = false;
    let isDragging = false;
    let dragStartX = 0, dragStartY = 0;

    btnMinimize.addEventListener('click', () => {
        windowFrame.style.transform = 'scale(0.96)';
        windowFrame.style.opacity = '0.5';
        setTimeout(() => {
            windowFrame.style.transform = 'scale(1)';
            windowFrame.style.opacity = '1';
        }, 300);
        addLog('[نافذة] تم تصغير النافذة إلى شريط المهام.', 'system');
    });

    btnMaximize.addEventListener('click', () => {
        if (!isMaximized) {
            windowFrame.style.maxWidth = '100%';
            windowFrame.style.maxHeight = '100%';
            windowFrame.style.height = '100vh';
            windowFrame.style.borderRadius = '0';
            isMaximized = true;
        } else {
            windowFrame.style.maxWidth = '1240px';
            windowFrame.style.maxHeight = '860px';
            windowFrame.style.height = '92vh';
            windowFrame.style.borderRadius = 'var(--radius-xl)';
            isMaximized = false;
        }
    });

    btnClose.addEventListener('click', () => {
        if (confirm('هل تريد إغلاق محرك CustomEngine؟')) {
            windowFrame.style.opacity = '0';
            windowFrame.style.transform = 'scale(0.9)';
            setTimeout(() => {
                document.body.innerHTML = `
                    <div style="text-align:center; color:#94a3b8; font-family:'Tajawal',sans-serif;">
                        <h2 style="color:#38bdf8; margin-bottom:8px;">تم إيقاف تشغيل المحرك بنجاح</h2>
                        <p>يمكنك إعادة تشغيل الواجهة في أي وقت عبر تشغيل خادم المحرك المحلي.</p>
                    </div>
                `;
            }, 300);
        }
    });

    // 2. خوارزمية حساب المجموع الاختباري (CRC32 Checksum)
    function calculateCRC32(str) {
        let crc = 0 ^ (-1);
        for (let i = 0; i < str.length; i++) {
            crc = (crc >>> 8) ^ crcTable[(crc ^ str.charCodeAt(i)) & 0xFF];
        }
        return (crc ^ (-1)) >>> 0;
    }

    const crcTable = [];
    for (let n = 0; n < 256; n++) {
        let c = n;
        for (let k = 0; k < 8; k++) {
            c = ((c & 1) ? (0xEDB88320 ^ (c >>> 1)) : (c >>> 1));
        }
        crcTable[n] = c;
    }

    // 3. خوارزمية Token Bucket لتحديد سرعة التدفق لكل ملف
    class TokenBucket {
        constructor(rateBytesPerSec = 0) {
            this.setRate(rateBytesPerSec);
            this.tokens = this.capacity;
            this.lastCheck = performance.now();
        }

        setRate(rateBytesPerSec) {
            this.rate = rateBytesPerSec; // 0 تعني غير محدود
            this.capacity = rateBytesPerSec > 0 ? Math.max(rateBytesPerSec, 32768) : 1e9;
        }

        tryConsume(bytes) {
            if (this.rate <= 0) return true; // سرعة غير محدودة
            const now = performance.now();
            const elapsed = (now - this.lastCheck) / 1000.0;
            this.lastCheck = now;

            this.tokens = Math.min(this.capacity, this.tokens + elapsed * this.rate);

            if (this.tokens >= bytes) {
                this.tokens -= bytes;
                return true;
            }
            return false;
        }
    }

    // 4. بنك الملفات التجريبية المستوحاة من اللعبة
    const mockFiles = [
        {
            id: 101,
            name: "Level1_TrackGeometry.mesh",
            category: "Mesh",
            totalBytes: 384000, // 384 KB
            chunkSize: 1160,    // حجم الجزء بعد طرح الترويسة
            colorBadge: "3D",
            speedLimit: 256 * 1024 // سقف افتراضي 256 KB/s
        },
        {
            id: 102,
            name: "WaterSimulation_Vfx.bundle",
            category: "VFX",
            totalBytes: 740000, // 740 KB
            chunkSize: 1160,
            colorBadge: "FX",
            speedLimit: 512 * 1024 // 512 KB/s
        },
        {
            id: 103,
            name: "PirateCustomSaveState.dat",
            category: "Save",
            totalBytes: 96000,  // 96 KB
            chunkSize: 1160,
            colorBadge: "DB",
            speedLimit: 128 * 1024 // 128 KB/s
        },
        {
            id: 104,
            name: "AudioPuffSounds_Library.bank",
            category: "Sound",
            totalBytes: 512000, // 512 KB
            chunkSize: 1160,
            colorBadge: "AU",
            speedLimit: 0 // Unlimited
        }
    ];

    // إعداد حالات البث وإعادة التجميع لكل ملف
    const fileStates = new Map();
    const filesList = document.getElementById('filesList');

    mockFiles.forEach(file => {
        const totalChunks = Math.ceil(file.totalBytes / file.chunkSize);
        const tokenBucket = new TokenBucket(file.speedLimit);

        fileStates.set(file.id, {
            ...file,
            totalChunks,
            sentChunks: 0,
            receivedChunks: 0,
            tokenBucket,
            isRunning: false,
            isComplete: false,
            reassemblyBuffer: new Array(totalChunks).fill(null),
            activeSpeedKB: 0,
            lastSentBytes: 0,
            lastSpeedCheck: performance.now()
        });

        // توليد واجهة كارت الملف
        const card = document.createElement('div');
        card.className = 'file-card';
        card.id = `fileCard_${file.id}`;
        card.innerHTML = `
            <div class="file-card-top">
                <div class="file-meta">
                    <div class="file-badge">${file.colorBadge}</div>
                    <div>
                        <div class="file-title">${file.name}</div>
                        <div class="file-sub">الحجم: ${(file.totalBytes / 1024).toFixed(1)} KB | الأجزاء: ${totalChunks} Chunks (MTU 1200)</div>
                    </div>
                </div>
                <span class="file-status-badge status-active" id="badge_${file.id}">جاهز</span>
            </div>

            <div class="progress-container">
                <div class="progress-info">
                    <span id="chunksText_${file.id}">0 / ${totalChunks} أجزاء</span>
                    <span id="percentText_${file.id}">0%</span>
                </div>
                <div class="progress-bar-bg">
                    <div class="progress-bar-fill" id="fill_${file.id}"></div>
                </div>
            </div>

            <div class="rate-limit-control">
                <div class="limit-label">
                    <svg viewBox="0 0 24 24" width="14" height="14" fill="none" stroke="currentColor" stroke-width="2">
                        <circle cx="12" cy="12" r="10"></circle>
                        <polyline points="12 6 12 12 16 14"></polyline>
                    </svg>
                    <span>سرعة التدفق المسموحة:</span>
                </div>
                <input type="range" class="speed-slider" id="slider_${file.id}" min="0" max="2048" step="64" value="${file.speedLimit / 1024}">
                <div class="limit-display" id="limitDisplay_${file.id}">
                    ${file.speedLimit === 0 ? 'غير محدود' : (file.speedLimit / 1024) + ' KB/s'}
                </div>
            </div>
        `;
        filesList.appendChild(card);

        // ربط شريط تمرير تحديد السرعة (Slider)
        const slider = document.getElementById(`slider_${file.id}`);
        const limitDisplay = document.getElementById(`limitDisplay_${file.id}`);

        slider.addEventListener('input', (e) => {
            const valKB = parseInt(e.target.value);
            const state = fileStates.get(file.id);
            if (valKB === 0) {
                state.tokenBucket.setRate(0);
                limitDisplay.textContent = 'غير محدود';
                addLog(`[سقف السرعة] ملف ${file.name} أصبح غير محدود السرعة.`, 'system');
            } else {
                const bytesSec = valKB * 1024;
                state.tokenBucket.setRate(bytesSec);
                limitDisplay.textContent = `${valKB} KB/s`;
                addLog(`[سقف السرعة] ملف ${file.name} تم ضبط سرعته على ${valKB} KB/s.`, 'system');
            }
        });
    });

    // 5. محرك دورات التحديث (Engine Tick Loop & Scheduler)
    let globalNetworkTick = 0;
    let totalProcessedChunks = 0;
    let isSimulationRunning = false;

    const elNetworkTick = document.getElementById('networkTick');
    const elGlobalSpeed = document.getElementById('globalSpeed');
    const elTotalChunks = document.getElementById('totalChunksProcessed');
    const logConsole = document.getElementById('logConsole');

    function addLog(text, type = 'chunk') {
        const entry = document.createElement('div');
        entry.className = `log-entry ${type}`;
        const time = new Date().toLocaleTimeString('ar-EG', { hour12: false });
        entry.textContent = `[${time}] ${text}`;
        logConsole.appendChild(entry);
        logConsole.scrollTop = logConsole.scrollHeight;

        // الحد الأقصى للأعطال والسطور في الكونسول لتوفير الذاكرة
        if (logConsole.children.length > 50) {
            logConsole.removeChild(logConsole.children[0]);
        }
    }

    // دورة الإرسال والدمج (Tick Step)
    function engineLoop() {
        if (isSimulationRunning) {
            globalNetworkTick++;
            elNetworkTick.textContent = globalNetworkTick;

            let tickTotalBytes = 0;

            fileStates.forEach((state, fileId) => {
                if (state.isRunning && !state.isComplete) {
                    
                    // محاولة إرسال حزمة جزئية واحدة في هذه الدورة وفق رصيد Token Bucket
                    const currentChunkIdx = state.sentChunks;
                    const isLastChunk = (currentChunkIdx === state.totalChunks - 1);
                    const chunkSize = isLastChunk ? (state.totalBytes - (currentChunkIdx * state.chunkSize)) : state.chunkSize;

                    if (state.tokenBucket.tryConsume(chunkSize)) {
                        state.sentChunks++;
                        totalProcessedChunks++;
                        tickTotalBytes += chunkSize;

                        // محاكاة الإرسال واستخراج CRC32 للجزء
                        const chunkPayloadFake = `CHUNK_DATA_${fileId}_${currentChunkIdx}_${chunkSize}`;
                        const crc = calculateCRC32(chunkPayloadFake);

                        // محاكاة الاستقبال الآمن عبر SplitReader
                        state.reassemblyBuffer[currentChunkIdx] = {
                            index: currentChunkIdx,
                            size: chunkSize,
                            crc: crc
                        };
                        state.receivedChunks++;

                        // تحديث واجهة المستخدم للكارت
                        const progress = (state.receivedChunks / state.totalChunks) * 100;
                        document.getElementById(`fill_${fileId}`).style.width = `${progress}%`;
                        document.getElementById(`percentText_${fileId}`).textContent = `${Math.round(progress)}%`;
                        document.getElementById(`chunksText_${fileId}`).textContent = `${state.receivedChunks} / ${state.totalChunks} أجزاء`;

                        // التحقق الذري من الاكتمال دون أي تلف
                        if (state.receivedChunks === state.totalChunks) {
                            state.isComplete = true;
                            state.isRunning = false;
                            const badge = document.getElementById(`badge_${fileId}`);
                            badge.className = 'file-status-badge status-complete';
                            badge.textContent = '✓ مكتمل وسليم 100%';

                            addLog(`[SplitReader] اكتمل دمج ملف ${state.name} بالكامل (${(state.totalBytes / 1024).toFixed(1)} KB) دون أي تلف (CRC32 Verified).`, 'complete');
                        }
                    }
                }
            });

            // حساب السرعة الإجمالية الحالية بالكيلوبايت/ثانية
            const speedKB = (tickTotalBytes * 60) / 1024; // بمعدل 60 دورة/ثانية
            elGlobalSpeed.textContent = speedKB.toFixed(1);
            elTotalChunks.textContent = totalProcessedChunks;
        }

        requestAnimationFrame(engineLoop);
    }

    requestAnimationFrame(engineLoop);

    // أزرار تشغيل وإيقاف البث
    const btnStartAll = document.getElementById('btnStartAll');
    const btnPauseAll = document.getElementById('btnPauseAll');

    btnStartAll.addEventListener('click', () => {
        isSimulationRunning = true;
        fileStates.forEach(s => {
            if (!s.isComplete) s.isRunning = true;
        });
        addLog('[النواة] تم تفعيل بث وجدولة تدفق كافة الحزم.', 'system');
    });

    btnPauseAll.addEventListener('click', () => {
        isSimulationRunning = false;
        fileStates.forEach(s => s.isRunning = false);
        addLog('[النواة] تم إيقاف بث وجدولة الحزم مؤقتاً.', 'warn');
        elGlobalSpeed.textContent = "0.0";
    });

    // 6. محاكي عصا التحكم والمدخلات (Input Manager Simulator)
    const joystickZone = document.getElementById('joystickZone');
    const joystickKnob = document.getElementById('joystickKnob');
    const moveVal = document.getElementById('moveVal');
    const btnGas = document.getElementById('btnGas');
    const btnPuff = document.getElementById('btnPuff');

    let isKnobDragging = false;

    function handleJoystickMove(e) {
        const rect = joystickZone.getBoundingClientRect();
        const centerX = rect.width / 2;
        const centerY = rect.height / 2;

        const clientX = e.clientX || (e.touches && e.touches[0].clientX);
        const clientY = e.clientY || (e.touches && e.touches[0].clientY);

        const deltaX = clientX - (rect.left + centerX);
        const deltaY = clientY - (rect.top + centerY);

        const dist = Math.min(45, Math.hypot(deltaX, deltaY));
        const angle = Math.atan2(deltaY, deltaX);

        const knobX = Math.cos(angle) * dist;
        const knobY = Math.sin(angle) * dist;

        joystickKnob.style.transform = `translate(${knobX}px, ${knobY}px)`;

        const normX = (knobX / 45).toFixed(2);
        const normY = (-knobY / 45).toFixed(2);
        moveVal.textContent = `[${normX}, ${normY}]`;
    }

    joystickZone.addEventListener('mousedown', (e) => {
        isKnobDragging = true;
        handleJoystickMove(e);
    });

    window.addEventListener('mousemove', (e) => {
        if (isKnobDragging) handleJoystickMove(e);
    });

    window.addEventListener('mouseup', () => {
        if (isKnobDragging) {
            isKnobDragging = false;
            joystickKnob.style.transform = 'translate(0px, 0px)';
            moveVal.textContent = '[0.00, 0.00]';
        }
    });

    btnGas.addEventListener('mousedown', () => {
        btnGas.classList.add('active');
        addLog('[مدخلات] تم إرسال أمر تسارع NormalGas (أولوية قصوى).', 'system');
    });
    btnGas.addEventListener('mouseup', () => btnGas.classList.remove('active'));

    btnPuff.addEventListener('mousedown', () => {
        btnPuff.classList.add('active');
        addLog('[مدخلات] تم إرسال أمر نفخ الغاز MousePuff (أولوية قصوى).', 'system');
    });
    btnPuff.addEventListener('mouseup', () => btnPuff.classList.remove('active'));

    // تشغيل تلقائي للبث عند التحميل
    setTimeout(() => {
        btnStartAll.click();
    }, 600);
});
