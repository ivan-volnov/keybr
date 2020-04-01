# keybr
**keybr** is an advanced console keyboard trainer with language learning support. Just import your learning cards from [Anki](https://apps.ankiweb.net) and then use them for touch typing training. The app will track your typing speed and errors and choose phrases for repeating to force learning process. It also can read aloud current phrase.

![screen](https://raw.githubusercontent.com/ivan-volnov/keybr/master/img/screen.png)

It supports only **macOS**. Other operating systems was not tested and aren't planned.

### Build:
```
mkdir build
cd build/
cmake ..
make install
```
It installs into /usr/local/bin directory

### Anki preparation:
- Install [AnkiConnect](https://ankiweb.net/shared/info/2055492159) plugin
- Check your Note Type Fields. They must be **Front** and **Back**

### Import cards from Anki with custom query:
```
keybr --import --anki_query "\"deck:En::Vocabulary Profile\" is:due -is:new -is:suspended"
```
Use [Anki Searching Query Language](https://docs.ankiweb.net/#/searching). Before importing you can test the query in Anki's Browse screen

### Run:
```
keybr
```

Press escape to exit

### Run with Speech Engine enabled:
```
keybr --sound
```
The app will read aloud the current phrase while typing
