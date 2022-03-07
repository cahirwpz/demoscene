# Prerequisites:

* Zakład font (https://github.com/warszawskie-kroje/zaklad)
* Lombard font (https://github.com/warszawskie-kroje/lombard)
* InkScape 1.0 or newer (maybe can be replaced with InkScape)
* ImageMagick

# Procedure:

Use InkScape export PNG with antialiasing DISABLED.
It will not prevent it from smothering the diagonal line because why not? In this image you can remove the additional colours by using:
convert background.png -fill white -opaque grey98 background.png
convert background.png -fill white -opaque '#FEFEFEFF' background.png
You can find such close colours you can potentially reduce with identify -verbose

Then you should convert the image to the format requested by PNG2C:
convert background.png PNG8:clock-bg.png
