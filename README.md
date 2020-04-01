# keybr
**keybr** is an advanced console keyboard trainer with language learning support. Just import your learning cards from [Anki](https://apps.ankiweb.net) and then use them for touch typing training. The app will track your typing speed and errors and choose phrases for repeating to force learning process. It also can read aloud current phrase.

It supports only macOS. Other Operating Systems was not tested and aren't planned.

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

### Import cards from Anki:
```
keybr --import
```

### Run:
```
keybr
```

### Run with Speech Engine enabled:
```
keybr --sound
```
The app will read the current word
