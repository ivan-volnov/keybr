# keybr
**keybr** is an advanced console keyboard trainer with language learning support. Just import your learning cards from [Anki](https://apps.ankiweb.net) and then use them for touch typing training. The app will track your typing errors and speed and choose phrases for repeating to force learning process.

### Build:
```
mkdir build
cd build/
cmake ..
make install
```

### Anki preparation:
- Install [AnkiConnect](https://ankiweb.net/shared/info/2055492159) plugin
- Check your Note Type Fields. They must be **Front** and **Back**

### Run:
```
# import cards from Anki
keybr --import

# run the app
keybr
```

### Run with Speech Engine enabled:
```
keybr --sound
```
The app will read the current word
