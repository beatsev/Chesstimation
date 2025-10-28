# BLE + USB Only Migration Plan
**Date**: October 22, 2025
**Goal**: Remove BT Classic stack, keep only BLE + USB
**Expected Memory Savings**: 30-40KB RAM

## Rationale

**Why remove BT Classic:**
- Legacy technology (deprecated in favor of BLE)
- Consumes 30-40KB RAM continuously
- User confirmed: "BLE only is the non-legacy way, BT-only is never required"
- USB already worked previously
- Simplifies codebase and reduces maintenance burden

**What we keep:**
- ✅ **USB**: Simple, works for desktop chess software, zero RAM overhead
- ✅ **BLE**: Modern standard, works with mobile apps (WhitePawn iOS, Chess for Android)

## Current Code Analysis

### BT Classic Usage (TO BE REMOVED)
```cpp
// Line 29: Include
#include <BluetoothSerial.h>

// Line 61: Global object
BluetoothSerial SerialBT;

// Line 66: Enum member
enum connectionType {USB, BT, BLE} connection;
                           ^^^
// Lines 176, 208, 384, 385: SerialBT writes
SerialBT.print(message);
SerialBT.println(message);
SerialBT.print(codedMessage.c_str());
SerialBT.flush();

// Lines 611-638: Initialization
SerialBT.begin("Chesstimation DEBUG");
SerialBT.begin("Certabo");
SerialBT.begin("MILLENNIUM CHESS BT");

// Lines 1781-1818: Reading data
while (SerialBT.available())
  led_buffer[writeToIndex] = SerialBT.read();
SerialBT.readBytes(&readChar, 1);
```

### USB Usage (KEEP)
```cpp
// Standard Arduino Serial - already in use
Serial.print(codedMessage.c_str());
Serial.available();
Serial.read();
Serial.readBytes(&readChar, 1);
```

### BLE Usage (KEEP)
```cpp
// Lines 79-81: BLE objects
BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic;
BLEService *pService;

// Lines 372-374: BLE notify
pTxCharacteristic->setValue(codedMessage.substr(i, 8));
pTxCharacteristic->notify();

// Lines 417-429: BLE callbacks
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    connection = BLE;
  }
  void onDisconnect(BLEServer *pServer) {
    millBLEinitialized = 0;
  }
};
```

## Implementation Steps

### Step 1: Update Connection Enum
**File**: `src/main.cpp:66`

**Before**:
```cpp
enum connectionType {USB, BT, BLE} connection;
```

**After**:
```cpp
enum connectionType {USB, BLE} connection;
```

**Impact**: All `connection == BT` checks will need updating

---

### Step 2: Remove BluetoothSerial Include and Object
**File**: `src/main.cpp`

**Remove lines**:
```cpp
// Line 29
#include <BluetoothSerial.h>

// Line 61
BluetoothSerial SerialBT;
```

**Memory saved**: ~35KB (BT Classic stack)

---

### Step 3: Remove BT-Specific Code Paths

#### 3a. Remove from `debugPrint()` function
**Location**: `src/main.cpp:167-180`

**Before**:
```cpp
byte debugPrint(const char *message)
{
  if(connection != USB)
  {
    Serial.print(message);
    return 1;
  }
  if(connection != BT)
  {
    SerialBT.print(message);
    return 1;
  }
  return 0;
}
```

**After**:
```cpp
byte debugPrint(const char *message)
{
  if(connection == USB)
  {
    Serial.print(message);
    return 1;
  }
  // BLE doesn't use debug prints (uses characteristics)
  return 0;
}
```

#### 3b. Remove from `debugPrintln()` function
**Location**: `src/main.cpp:199-211`

**Before**:
```cpp
byte debugPrintln(const char *message)
{
  if(connection != USB)
  {
    Serial.println(message);
    return 1;
  }
  if(connection != BT)
  {
    SerialBT.println(message);
    return 1;
  }
  return 0;
}
```

**After**:
```cpp
byte debugPrintln(const char *message)
{
  if(connection == USB)
  {
    Serial.println(message);
    return 1;
  }
  return 0;
}
```

#### 3c. Remove from `saveBoardSettings()` function
**Location**: `src/main.cpp:224-231`

**Before**:
```cpp
if ((connection == BLE) && (chessBoard.emulation != 0))
{
  saveConnection = BLE;
}
if (connection == BT)
{
  saveConnection = BT;
}
```

**After**:
```cpp
if ((connection == BLE) && (chessBoard.emulation != 0))
{
  saveConnection = BLE;
}
// Only USB and BLE now, no BT
```

#### 3d. Remove from `sendMessage()` function
**Location**: `src/main.cpp:348-391`

**Before**:
```cpp
void sendMessage()
{
  // ... (lines omitted)

  if (connection == BLE)
  {
    // BLE code...
  }

  std::string codedMessage = std::string(chessBoard.boardMessage);

  if (connection == BT)
  {
    SerialBT.print(codedMessage.c_str());
    SerialBT.flush();
  }
  if (connection == USB)
  {
    Serial.print(codedMessage.c_str());
    Serial.flush();
  }
}
```

**After**:
```cpp
void sendMessage()
{
  // ... (lines omitted)

  if (connection == BLE)
  {
    // BLE code...
    return; // Exit after BLE send
  }

  // USB communication
  std::string codedMessage = std::string(chessBoard.boardMessage);
  Serial.print(codedMessage.c_str());
  Serial.flush();
}
```

#### 3e. Remove from `changeConnection()` function
**Location**: `src/main.cpp:609-648`

**Before**:
```cpp
void changeConnection(void)
{
  // ... initialization code ...

  if (connection == USB)
  {
    // SerialBT.end();
    debugPrintln("Switching connection to USB!");
  }
  if(connection == BT)
  {
    // SerialBT.end();
    SerialBT.begin("Chesstimation DEBUG");
    debugPrintln("Switching connection to BT!");
  }

  if (connection == USB)
  {
    // SerialBT.end();
    debugPrintln("Switching to USB connection!");
  }
  if(connection == BT)
  {
    if (chessBoard.emulation == 0)
    {
      // SerialBT.end();
      SerialBT.begin("Certabo");
      debugPrintln("Switching to Certabo BT!");
    }
    else
    {
      // SerialBT.end();
      SerialBT.begin("MILLENNIUM CHESS BT");
      debugPrintln("Switching to Chesslink BT!");
    }
  }
  if(connection == BLE)
  {
    // BLE code...
  }

  // ... rest of function ...
}
```

**After**:
```cpp
void changeConnection(void)
{
  // ... initialization code ...

  if (connection == USB)
  {
    debugPrintln("Switching connection to USB!");
  }

  if(connection == BLE)
  {
    if (chessBoard.emulation == 0)
    {
      debugPrintln("BLE only supports Chesslink emulation!");
      chessBoard.emulation = 1; // Force Chesslink for BLE
    }

    if(millBLEinitialized == 0)
    {
      initMilleniumBLE();
      millBLEinitialized = 1;
    }
    debugPrintln("Switching to Chesslink BLE!");
  }

  // ... rest of function ...
}
```

#### 3f. Remove from `loop()` - Certabo LED handling
**Location**: `src/main.cpp:1764-1790`

**Before**:
```cpp
if (connection == USB)
{
  while (Serial.available())
  {
    led_buffer[writeToIndex] = Serial.read();
    writeToIndex++;
    if (writeToIndex > 8)
    {
      writeToIndex = 0;
      Serial.println("L");
      Serial.flush();
    }
  }
}
if (connection == BT)
{
  while (SerialBT.available())
  {
    led_buffer[writeToIndex] = SerialBT.read();
    writeToIndex++;
    if (writeToIndex > 8)
    {
      writeToIndex = 0;
      SerialBT.println("L");
      SerialBT.flush();
    }
  }
}
```

**After**:
```cpp
if (connection == USB)
{
  while (Serial.available())
  {
    led_buffer[writeToIndex] = Serial.read();
    writeToIndex++;
    if (writeToIndex > 8)
    {
      writeToIndex = 0;
      Serial.println("L");
      Serial.flush();
    }
  }
}
// BLE uses characteristic-based communication, not serial
```

#### 3g. Remove from `loop()` - Chesslink message handling
**Location**: `src/main.cpp:1799-1823`

**Before**:
```cpp
if (connection == USB)
{
  if (Serial.available() > 0)
  {
    for (int i = 0; i < Serial.available(); i++)
    {
      Serial.readBytes(&readChar, 1);
      assembleIncomingChesslinkMessage(readChar);
    }
  }
}
if (connection == BT)
{
  if (SerialBT.available() > 0)
  {
    for (int i = 0; i < SerialBT.available(); i++)
    {
      SerialBT.readBytes(&readChar, 1);
      assembleIncomingChesslinkMessage(readChar);
    }
  }
}
```

**After**:
```cpp
if (connection == USB)
{
  if (Serial.available() > 0)
  {
    for (int i = 0; i < Serial.available(); i++)
    {
      Serial.readBytes(&readChar, 1);
      assembleIncomingChesslinkMessage(readChar);
    }
  }
}
// BLE message handling done via MyCallbacksChesslink::onWrite()
```

---

### Step 4: Update UI - Connection Selection

**Location**: Settings screen creation (around line 1400+)

**Current UI**: Dropdown with "USB / BT / BLE"

**Update to**: Dropdown with "USB / BLE"

**Search for**:
```cpp
connectionDd = lv_dropdown_create(settingsScreen);
lv_dropdown_set_options(connectionDd, "USB\nBT\nBLE");
```

**Replace with**:
```cpp
connectionDd = lv_dropdown_create(settingsScreen);
lv_dropdown_set_options(connectionDd, "USB\nBLE");
```

**Impact**:
- Index 0 = USB (unchanged)
- Index 1 = BLE (was 2 before)
- Need to adjust loading/saving connection value

---

### Step 5: Update SPIFFS Save/Load

**In `saveBoardSettings()`**:
```cpp
// Before saving connection:
byte saveConnection = connection;

// Adjust index for storage (keep compatibility)
if (connection == BLE) {
  saveConnection = 2; // Keep old BLE value for backward compatibility
}

f.write(saveConnection);
```

**In `loadBoardSettings()`**:
```cpp
// After loading connection:
byte loadedConnection = tempInt8[0];

// Map old values to new enum
if (loadedConnection == 0) connection = USB;      // 0 = USB
else if (loadedConnection == 1) connection = USB; // 1 = old BT, map to USB
else if (loadedConnection == 2) connection = BLE; // 2 = BLE
else connection = USB; // Default
```

---

### Step 6: Verify USB Serial Initialization

**Location**: `setup()` function

**Ensure USB Serial is initialized**:
```cpp
void setup() {
  Serial.begin(115200); // Should already exist

  // ... rest of setup ...
}
```

---

## Testing Plan

### Test 1: USB Communication
**Hardware**: Connect via USB-C cable
**Steps**:
1. Build and upload firmware
2. Select USB in connection settings
3. Open serial monitor at 115200 baud
4. Move pieces on board
5. Verify board messages are sent via USB Serial

**Expected**: Board state updates visible in serial monitor

---

### Test 2: BLE Communication
**Hardware**: BLE-capable device (phone/tablet)
**Steps**:
1. Select BLE in connection settings
2. Restart device
3. Pair with BLE device "MILLENNIUM" or "Certabo"
4. Connect via WhitePawn or Chess for Android app
5. Move pieces on board
6. Verify app receives board state

**Expected**: App shows board state correctly

---

### Test 3: Memory Usage
**Monitor free heap**:
```cpp
void setup() {
  Serial.begin(115200);
  Serial.printf("Free heap before init: %d\n", esp_get_free_heap_size());

  // ... initialization ...

  Serial.printf("Free heap after init: %d\n", esp_get_free_heap_size());
}
```

**Expected**: 30-40KB more free heap compared to BT Classic version

---

### Test 4: Settings Persistence
**Steps**:
1. Select USB, restart → verify USB still selected
2. Select BLE, restart → verify BLE still selected
3. Test with old SPIFFS data (backward compatibility)

**Expected**: Settings persist across reboots

---

## Code Changes Summary

### Files Modified
- `src/main.cpp` (primary changes)

### Lines to Remove
- Line 29: `#include <BluetoothSerial.h>`
- Line 61: `BluetoothSerial SerialBT;`
- All `SerialBT.*` method calls (~15 locations)
- All `if (connection == BT)` blocks (~8 locations)

### Lines to Modify
- Line 66: `enum connectionType {USB, BLE}`
- Connection dropdown options
- `debugPrint()` and `debugPrintln()` functions
- `sendMessage()` function
- `changeConnection()` function
- SPIFFS load/save functions
- Main `loop()` serial handling

### Estimated Changes
- **Lines deleted**: ~50
- **Lines modified**: ~30
- **Net reduction**: ~40 lines of code
- **Complexity reduction**: 1 less communication stack

---

## Risks and Mitigation

### Risk 1: Backward Compatibility
**Issue**: Users with saved BT connection setting
**Mitigation**: Map old BT (value 1) to USB in load function

### Risk 2: BLE-Only for Chesslink
**Issue**: BLE only supports Chesslink emulation (not Certabo)
**Mitigation**: Force Chesslink emulation when BLE selected (already in code at line 224)

### Risk 3: Missing BT functionality
**Issue**: Some users may have used BT Classic
**Mitigation**:
- USB provides wired alternative
- BLE provides wireless alternative
- Both cover all use cases

### Risk 4: Build errors from missing BluetoothSerial
**Issue**: May have compilation issues
**Mitigation**: Systematic removal of all references before building

---

## Success Criteria

- [ ] Code compiles without errors
- [ ] USB serial communication works (tested with serial monitor)
- [ ] BLE communication works (tested with WhitePawn or Chess app)
- [ ] Settings screen shows only USB/BLE options
- [ ] Settings persist across reboots
- [ ] Free heap increased by 30-40KB
- [ ] No .dram0.data overflow errors
- [ ] Display functionality unchanged
- [ ] All existing chess features work (piece detection, LED feedback, etc.)

---

## Next Steps After Migration

Once BT Classic is removed and memory is freed:

1. **Implement Chess Clock** (MEMORY_ANALYSIS.md plan)
2. **Add memory monitoring utilities**
3. **Test with actual chess applications**
4. **Update README.md** to reflect BLE+USB only
5. **Update CLAUDE.md** with new architecture

---

## Rollback Plan

If issues arise:
1. Git revert changes
2. Return to current BT+BLE+USB version
3. Analyze specific failure
4. Create targeted fix

**Git branch strategy**: Create `feature/ble-usb-only` branch before changes

---

## Documentation Updates Required

After successful migration:

1. **README.md**: Update connection methods section
2. **CLAUDE.md**: Update communication layer description
3. **MEMORY_ANALYSIS.md**: Update with actual memory savings
4. **DEVELOPMENT_PLAN.md**: Mark BT removal as complete
5. **PRD.md**: Add BT removal to changelog

---

## Implementation Timeline

**Estimated time**: 2-3 hours

- **Hour 1**: Remove BT code, update enum
- **Hour 2**: Test USB and BLE communication
- **Hour 3**: Update UI, SPIFFS, documentation

**Ready to proceed with implementation?**
