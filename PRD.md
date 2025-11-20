# Product Requirements Document

This document outlines the requirements for the Chesstimation project. It is a living document that will be updated as the project evolves.

## 1. Underpromotion

- [x] **UI for Promotion Selection:**
    - [x] Create a new UI component in `src/main.cpp` using the LVGL library to display the promotion options (Queen, Rook, Bishop, or Knight).
    - [x] The UI component should appear when a pawn reaches the promotion rank.
    - [x] The UI component should be intuitive and easy to use.

- [x] **Logic for Handling Promotion:**
    - [x] Modify the `setPieceBackTo` function in `src/board.cpp` to handle the promotion selection.
    - [x] The function should wait for the user's selection from the UI.
    - [x] Based on the selection, the appropriate piece should be placed on the board.

- [x] **Communication Protocol:**
    - [x] Update the communication protocol to send the promotion information to the connected chess program.
    - [x] Modify the `generateSerialBoardMessage` function in `src/board.cpp` to include the promotion information in the message.


## 2. Chess Clock Integration

### Overview
Add integrated chess clock functionality to transform Chesstimation into a complete over-the-board chess solution. The chess clock must operate within strict ESP32 memory constraints while providing tournament-grade time control features.

### User Stories
- **As a chess player**, I want a visual chess clock on the display so I can play timed games without external clock hardware.
- **As a tournament organizer**, I want standard time controls (Fischer, Bronstein, Simple Delay) so the device supports official tournament formats.
- **As a mobile user**, I want the clock to persist through power cycles so I can pause and resume games without losing time.
- **As a developer**, I want the clock to integrate with existing protocols so chess software can sync time information.

---

### 2.1 UI/UX Requirements

#### 2.1.1 Display Layout
- [ ] **Timer Display Location:**
    - [ ] Primary location: Replace debug label area at position (327, 225) with dual timer display
    - [ ] Size: 146×50 pixels (expand debug area vertically from 25px to 50px)
    - [ ] Layout: Horizontal split - White timer (left 73px) | Black timer (right 73px)
    - [ ] Fallback: If more space needed, use area (327, 0) to (480, 60) above battery indicator

- [ ] **Visual Design:**
    - [ ] Font: Reuse existing `montserrat_umlaute20` or `montserrat_umlaute22` fonts (memory-efficient)
    - [ ] Color coding:
        - [ ] Active player timer: Green text or highlighted background
        - [ ] Inactive player timer: White/gray text
        - [ ] Time warning (<30s): Yellow text
        - [ ] Time critical (<10s): Red text with blinking effect
    - [ ] Display format: `MM:SS` for times >60s, `SS.S` (deciseconds) for times <60s
    - [ ] Current turn indicator: Small triangle or border highlight on active timer

#### 2.1.2 Touch Controls
- [ ] **Clock Activation:**
    - [ ] Add "Chess Clock" toggle button in Settings screen (below "Brightness" slider)
    - [ ] When enabled, timer display replaces debug label on main screen
    - [ ] Toggle persists in SPIFFS `/board_setup` file

- [ ] **In-Game Controls:**
    - [ ] Touch timer area to pause/resume (requires confirmation to prevent accidental taps)
    - [ ] Touch active timer switches turn to opponent (standard chess clock behavior)
    - [ ] Long-press timer area (2 seconds) to open clock settings overlay

- [ ] **Clock Settings Overlay:**
    - [ ] Modal dialog accessible via long-press on timer display
    - [ ] Options:
        - [ ] Time control preset selection (dropdown/list)
        - [ ] Custom time configuration (hours, minutes, seconds input)
        - [ ] Increment/delay amount (seconds input)
        - [ ] Reset clock button
        - [ ] Start new game button (resets board and clock)
    - [ ] Overlay size: 280×200 pixels (centered on screen)
    - [ ] Reuse existing LVGL modal box component from promotion screen

#### 2.1.3 Time Control Presets
- [ ] **Standard Presets:**
    - [ ] Bullet: 1+0, 2+1
    - [ ] Blitz: 3+0, 3+2, 5+0, 5+3
    - [ ] Rapid: 10+0, 15+10, 25+10
    - [ ] Classical: 60+30, 90+30
    - [ ] Custom: User-defined time + increment

- [ ] **Increment Types:**
    - [ ] Fischer (add time after move completion)
    - [ ] Bronstein (add time up to original think time)
    - [ ] Simple Delay (countdown starts after delay expires)
    - [ ] None (no increment)

---

### 2.2 Functional Requirements

#### 2.2.1 Core Clock Logic
- [ ] **Timer Engine:**
    - [ ] Implement timer using FreeRTOS software timers (`xTimerCreate()`)
    - [ ] Update frequency: 100ms for general use, 10ms when <10s remaining
    - [ ] Precision: ±50ms acceptable drift over 1-hour game
    - [ ] Handle timer interrupts without blocking LVGL display updates

- [ ] **Game Flow Integration:**
    - [ ] Automatically start opponent's clock when piece is placed (move detected)
    - [ ] Pause clock when piece is lifted and held (piece in hand >2 seconds)
    - [ ] Resume clock when piece placed completes move
    - [ ] Option to disable auto-switching (manual tap mode)

- [ ] **Time Expiration:**
    - [ ] Flag fallen indicator when time reaches 00:00.0
    - [ ] Continue counting negative time (show as "-MM:SS" in red)
    - [ ] Display full-screen "Time Expired - [White/Black] Wins" overlay
    - [ ] Require manual acknowledgment to dismiss overlay

#### 2.2.2 State Persistence
- [ ] **SPIFFS Storage:**
    - [ ] Save clock state to `/clock_state` file every 10 seconds while running
    - [ ] Store: White time remaining, Black time remaining, active player, time control settings
    - [ ] File format: Binary struct (16 bytes max) or JSON (if space permits)
    - [ ] Restore clock on boot if saved state exists and <24 hours old

- [ ] **Power Management:**
    - [ ] Pause clock on sleep/shutdown (display overlay: "Clock Paused - Touch to Resume")
    - [ ] Resume clock on wake if game was active
    - [ ] Warn user if attempting to sleep during active game (confirmation dialog)

#### 2.2.3 Audio/Visual Feedback
- [ ] **Warning Alerts (no hardware speaker available):**
    - [ ] Visual pulse: Flash timer background when <30s, <10s, <5s thresholds crossed
    - [ ] Haptic feedback: Use TFT backlight rapid dim/bright pulse as "vibration" substitute
    - [ ] LED feedback: Flash board LEDs in pattern (if not conflicting with move display)

- [ ] **Move Confirmation:**
    - [ ] Brief backlight flash when clock switches sides (visual "click" feedback)

---

### 2.3 Technical Specifications

#### 2.3.1 Memory Budget
- [ ] **RAM Allocation Analysis:**
    - [ ] Timer state struct: 32 bytes (2× uint32_t time, 2× uint16_t increment, flags)
    - [ ] FreeRTOS timer handle: ~96 bytes (system allocation)
    - [ ] LVGL objects (reuse existing labels): 0 bytes additional (modify debugLbl)
    - [ ] Clock settings overlay: ~512 bytes (temporary heap allocation when displayed)
    - [ ] **Total estimated: <1KB DRAM impact**

- [ ] **Optimization Strategies:**
    - [ ] Use existing `debugLbl` object instead of creating new label (0 bytes)
    - [ ] Store time control presets in PROGMEM (flash) as const arrays
    - [ ] Allocate settings overlay on heap only when displayed, free on close
    - [ ] Reuse existing dropdown components from settings screen

- [ ] **Memory Monitoring:**
    - [ ] Add `esp_get_free_heap_size()` check before enabling clock
    - [ ] Require >10KB free heap to enable chess clock feature
    - [ ] Display warning if memory low: "Disable Bluetooth to enable Clock"

#### 2.3.2 Code Architecture
- [ ] **File Structure:**
    - [ ] Create `src/chessclock.h` and `src/chessclock.cpp` for clock logic
    - [ ] Class: `ChessClock` with methods: `start()`, `pause()`, `switchPlayer()`, `setTimeControl()`, `getTimeRemaining()`
    - [ ] Integration: Call from `main.cpp` in existing game loop

- [ ] **API Design:**
    ```cpp
    class ChessClock {
      public:
        ChessClock();
        void setTimeControl(uint16_t minutes, uint8_t seconds, uint8_t increment, IncrementType type);
        void start();
        void pause();
        void reset();
        void switchPlayer();
        uint32_t getTimeRemaining(bool whitePlayer); // milliseconds
        bool isExpired(bool whitePlayer);
        void update(); // Called from timer callback
      private:
        uint32_t whiteTimeMs;
        uint32_t blackTimeMs;
        bool isWhiteTurn;
        bool isPaused;
        uint8_t incrementSec;
        IncrementType incType;
        TimerHandle_t timerHandle;
    };
    ```

#### 2.3.3 Display Integration
- [ ] **LVGL Object Reuse:**
    - [ ] Modify existing `debugLbl` object for timer display (line 1465 in main.cpp)
    - [ ] Use `lv_label_set_text_fmt(debugLbl, "%02d:%02d | %02d:%02d", ...)` for dual timer
    - [ ] Apply color changes via `lv_obj_set_style_text_color()` for warnings
    - [ ] No new objects created - **zero LVGL memory overhead**

- [ ] **Update Strategy:**
    - [ ] Clock updates triggered by FreeRTOS timer callback
    - [ ] Callback sets flag, main loop updates display (avoid LVGL thread issues)
    - [ ] Use existing `loop()` function pattern (line 1722 in main.cpp)

---

### 2.4 Integration Requirements

#### 2.4.1 Protocol Support
- [ ] **Certabo Protocol Extension:**
    - [ ] Research if Certabo protocol supports clock sync messages
    - [ ] If yes: Send clock updates in format `CLOCK:W:MM:SS:B:MM:SS\n` via serial/BLE
    - [ ] If no: Document as "clock display only" feature (no sync)

- [ ] **Chesslink/Millennium Protocol:**
    - [ ] Investigate Millennium ChessLink clock commands
    - [ ] Implement bidirectional sync if protocol supports it (chess app controls clock)
    - [ ] Test with WhitePawn app to verify compatibility

- [ ] **Fallback Behavior:**
    - [ ] If protocol doesn't support clocks, operate in standalone mode
    - [ ] Display notification: "Clock is local only - not synced with app"

#### 2.4.2 Board Integration
- [ ] **Move Detection Sync:**
    - [ ] Hook into existing `Board::updatePiecePosition()` (board.cpp)
    - [ ] Call `chessClock.switchPlayer()` when valid move detected
    - [ ] Respect `Board::emulation` mode (Certabo vs Chesslink timing differences)

- [ ] **Settings Persistence:**
    - [ ] Extend `saveBoardSetup()` function (main.cpp line ~600) to include:
        - [ ] Clock enabled/disabled flag
        - [ ] Last used time control preset ID
    - [ ] Extend `loadBoardSetup()` to restore clock settings on boot

---

### 2.5 Hardware Considerations

#### 2.5.1 Limitations
- [ ] **No Audio Hardware:**
    - [ ] LOLIN D32 has no built-in speaker, buzzer, or piezo
    - [ ] Document requirement: "Audio alerts not supported - visual feedback only"
    - [ ] Future enhancement: Add GPIO pin documentation for optional external buzzer

- [ ] **Battery Impact:**
    - [ ] Measure power consumption of timer updates (estimated <5mA additional)
    - [ ] 2200mAh battery: Negligible impact (~3% reduction in runtime)
    - [ ] Display warning if battery <20% and clock active: "Low Battery - Connect USB"

#### 2.5.2 External Hardware (Optional)
- [ ] **Document GPIO Pin for Buzzer:**
    - [ ] Suggest GPIO 23 (currently unused) for future piezo buzzer
    - [ ] PWM frequency: 2700Hz for chess clock beep sound
    - [ ] Add conditional compilation flag: `#define ENABLE_CLOCK_BUZZER`

---

### 2.6 Testing & Validation

#### 2.6.1 Unit Tests
- [ ] **Timer Accuracy:**
    - [ ] Verify 1-minute countdown completes in 60.0 ± 0.5 seconds
    - [ ] Test timer behavior during Bluetooth transmission (no blocking)
    - [ ] Confirm timer continues during display updates (LVGL flush)

- [ ] **Increment Logic:**
    - [ ] Test Fischer increment adds correctly after move
    - [ ] Test Bronstein caps increment at original think time
    - [ ] Test Simple Delay countdown sequence

- [ ] **Edge Cases:**
    - [ ] Flag fall at exactly 00:00.000
    - [ ] Negative time accumulation after flag fall
    - [ ] Rapid clock taps (debounce to 500ms minimum)
    - [ ] Power cycle during active game (restore state)

#### 2.6.2 Integration Tests
- [ ] **Chess Application Compatibility:**
    - [ ] WhitePawn iOS app: Verify clock doesn't interfere with move sync
    - [ ] Lucas Chess: Test if protocol extensions are ignored gracefully
    - [ ] BearChess: Verify Certabo emulation compatibility
    - [ ] Chess for Android: Test BLE connection stability with clock updates

- [ ] **UI Interaction:**
    - [ ] Clock display updates while dragging pieces
    - [ ] Touch calibration unaffected by timer updates
    - [ ] Settings screen navigation works with clock running
    - [ ] Promotion dialog interaction with active clock

#### 2.6.3 Performance Tests
- [ ] **Memory Stability:**
    - [ ] Run clock for 2+ hours, monitor heap fragmentation
    - [ ] Verify no memory leaks: `esp_get_free_heap_size()` stable over time
    - [ ] Test with both BT Classic and BLE enabled simultaneously

- [ ] **Battery Life:**
    - [ ] Measure runtime with clock enabled vs disabled (target: <10% difference)
    - [ ] Test wake-from-sleep time with clock state restoration

- [ ] **Display Performance:**
    - [ ] Verify LVGL frame rate unchanged (target: 30fps maintained)
    - [ ] No screen flicker during timer updates
    - [ ] Touch responsiveness unaffected (<100ms latency)

---

### 2.7 Success Criteria

#### Must-Have (MVP)
- [ ] Timer displays and counts down accurately (±1 second over 10 minutes)
- [ ] Manual clock switching works via touch
- [ ] At least 3 time control presets available (Blitz, Rapid, Classical)
- [ ] State persists through power cycle
- [ ] Zero memory overflow errors during compilation
- [ ] Existing functionality 100% preserved (display, board reading, protocols)

#### Should-Have (V1.1)
- [ ] Automatic clock switching on move detection
- [ ] Fischer increment support
- [ ] Visual time warnings (<30s, <10s)
- [ ] Integration with at least one chess app protocol (clock sync)

#### Nice-to-Have (V2.0)
- [ ] Bronstein and Simple Delay increment types
- [ ] Historical game time tracking (SPIFFS log)
- [ ] Tournament mode (multiple time controls per game)
- [ ] External buzzer support via GPIO

---

### 2.8 Out of Scope

The following features are explicitly **not** included in the initial chess clock implementation:

- [ ] ❌ Online time sync via NTP/internet (requires WiFi, increases complexity)
- [ ] ❌ Multi-game tournament management (memory intensive)
- [ ] ❌ Time usage analytics/graphs (display size limitation)
- [ ] ❌ Voice announcements (no audio hardware)
- [ ] ❌ Smartphone app for clock control (protocol complexity)
- [ ] ❌ DGT clock protocol emulation (insufficient documentation)
- [ ] ❌ Video replay of time usage (storage constraint)

---

### 2.9 Development Phases

#### Phase 1: Foundation (Week 1)
- [ ] Implement `ChessClock` class with basic timer logic
- [ ] Add timer display to main screen (reuse debugLbl)
- [ ] Create settings toggle for clock enable/disable
- [ ] Implement manual clock switching via touch

#### Phase 2: Time Controls (Week 2)
- [ ] Add time control presets to settings overlay
- [ ] Implement Fischer increment logic
- [ ] Add SPIFFS persistence for clock state
- [ ] Create visual warning indicators

#### Phase 3: Integration (Week 3)
- [ ] Hook clock to board move detection (auto-switch)
- [ ] Implement protocol extensions (if supported)
- [ ] Add flag-fall overlay and negative time display
- [ ] Test with actual chess applications

#### Phase 4: Polish & Testing (Week 4)
- [ ] Memory optimization and leak testing
- [ ] Battery impact measurement
- [ ] User acceptance testing with chess players
- [ ] Documentation and usage guide updates

---

### 2.10 Risk Assessment

| Risk | Likelihood | Impact | Mitigation |
|------|------------|--------|------------|
| Memory overflow crashes | Medium | High | Implement feature flag for easy disable; monitor heap |
| Timer drift >5s over 1hr | Low | Medium | Use hardware timers instead of FreeRTOS; calibration |
| LVGL thread conflicts | Medium | High | Use flag-based update in main loop; avoid callback rendering |
| Protocol incompatibility | High | Low | Operate standalone if protocol doesn't support clocks |
| Battery drain >20% | Low | Medium | Reduce update frequency; use sleep modes aggressively |

---

## 3. WhitePawn iOS App Compatibility

- [x] **Investigate Communication Protocol:**
    - [x] Research the WhitePawn iOS app's communication protocol (e.g., OpenExchange Protocol, BLE characteristics, message formats).
    - [x] Analyze existing Chesstimation communication logic in `src/main.cpp` and `src/board.cpp` (Certabo, Chesslink emulations).
- [x] **Identify Compatibility:**
    - [x] Confirmed that WhitePawn app supports existing emulation protocols, eliminating the need for a separate WhitePawn emulation mode.
- [x] **Implementation Approach:**
    - [x] WhitePawn compatibility achieved through existing Chesslink/Millennium emulation mode.
    - [x] Removed unnecessary WhitePawn-specific emulation code to prevent bugs and maintain code simplicity.
    - [x] Users can connect WhitePawn app using the standard Chesslink/Millennium emulation mode.
- [ ] **Test Compatibility:**
    - [ ] Conduct thorough testing with the WhitePawn iOS app using Chesslink/Millennium emulation to ensure seamless communication and correct board state synchronization.


