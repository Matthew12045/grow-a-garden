# Title Screen Implementation Guide

## Overview
This title screen implementation for SFML3 includes:
- **Start Button** - Begins the game
- **Credits Button** - Displays scrolling credits
- **Exit Button** - Evasive button that tries to avoid the mouse cursor
- **Smooth Dimming/Brightening** - Screen fades in with dimming effect (like Terraria)
- **Hover Effects** - Text lights up yellow when hovering over buttons

## File Structure

```
src/title/
├── Button.h/cpp           - Individual button class with hover states
├── CreditsScreen.h/cpp    - Credits display with scrolling
├── TitleScreen.h/cpp      - Main title screen manager
```

## Features Implemented

### 1. **Screen Dimming & Brightening Effect**
- When the title screen loads, it starts fully dimmed (black overlay)
- Gradually brightens as the fade-in animation plays
- Similar to Terraria's title screen effect
- Takes ~2 seconds to fully brighten

### 2. **Interactive Buttons**
- **Three button states:** Normal, Hovered, Pressed
- Text changes color to yellow when hovered
- Rounded rectangle buttons with smooth colors
- Centered positioning on screen

### 3. **Evasive Exit Button**
- When mouse hovers near the exit button, it moves away
- Speed and direction based on mouse proximity
- Stops evading after 5 seconds of interaction
- Constrained within window bounds

### 4. **Credits Screen**
- Scrolling credits that auto-scroll upward
- Semi-transparent dark overlay
- "Press ESC to Return" instruction
- Auto-returns to title after scrolling completes
- Smooth fade-in effect

## Usage

### Setup

1. **Place your background image:**
   ```
   assets/textures/title_background.png
   ```
   - Size: 1920x1080 recommended (HD)
   - The image will be automatically scaled

2. **Place a font file:**
   ```
   assets/fonts/Arial.ttf
   ```
   - Any TTF font works
   - Required for button text rendering

### Compilation

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### Main Window Setup

The window is set to:
- **Resolution:** 1920x1080
- **Fullscreen:** Yes (can be changed in main.cpp)
- **FPS:** 60 (locked)

## Customization

### Adjust Button Colors

Edit `Button.cpp` constructor:
```cpp
normalColor = sf::Color(70, 100, 150, 200);      // Button normal state
hoveredColor = sf::Color(120, 150, 200, 255);    // Button hovered
pressedColor = sf::Color(150, 180, 220, 255);    // Button pressed
textNormalColor = sf::Color(200, 200, 200);      // Text normal
textHoveredColor = sf::Color(255, 255, 100);     // Text hovered (yellow)
```

### Adjust Fade Speed

Edit `TitleScreen.cpp`:
```cpp
fadeSpeed = 200.f;  // Increase for faster fade-in, decrease for slower
```

### Adjust Exit Button Evasion

Edit `TitleScreen.cpp` in `updateExitButtonEvasion()`:
```cpp
if (distance < 200.f) {  // Evasion trigger distance
    // ... 
    float newX = buttonCenter.x + std::cos(angle) * 50.f;  // Evasion distance
    //...
}
```

### Add Credits

Edit `TitleScreen::setFont()` or add a method to populate credits:
```cpp
void TitleScreen::addCredits() {
    creditsScreen->addCreditLine("Your Company", 0.f);
    creditsScreen->addCreditLine("Developers: Your Name", 100.f);
    // ... add more credits
}
```

## Integration with Main Game

Replace the TODO section in `main.cpp`:

```cpp
} else if (isGameRunning) {
    // Your game logic here
    // Update game state
    // Draw game
}
```

## Mouse Behavior

- **Left Click on Start:** Fades out and starts the game
- **Left Click on Credits:** Shows scrolling credits
- **Left Click on Exit:** Button evades, continues evading for 5 seconds
- **ESC while in Credits:** Returns to title screen

## Performance Notes

- Button rounding uses 50 points for smooth curves
- Fade calculations are lightweight
- Credits scrolling uses simple position updates
- Recommended for 60 FPS stable performance
