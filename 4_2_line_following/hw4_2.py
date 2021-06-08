import pyb, sensor, image, time, math
enable_lens_corr = False # turn on for straighter lines...
sensor.reset()
sensor.set_pixformat(sensor.GRAYSCALE) # grayscale is faster
sensor.set_framesize(sensor.QQQVGA) #80 * 60
sensor.skip_frames(time = 2000)
clock = time.clock()
THRESHOLD = (190, 255)
BINARY_VISIBLE = True

uart = pyb.UART(3,9600,timeout_char=1000)
uart.init(9600,bits=8,parity = None, stop=1, timeout_char=1000)

# All lines also have `x1()`, `y1()`, `x2()`, and `y2()` methods to get their end-points
# and a `line()` method to get all the above as one 4 value tuple for `draw_line()`.

while(True):
    clock.tick()
    img = sensor.snapshot().binary([THRESHOLD]) if BINARY_VISIBLE else sensor.snapshot()
    if enable_lens_corr: img.lens_corr(1.8) # for 2.8mm lens...

    # `merge_distance` controls the merging of nearby lines. At 0 (the default), no
    # merging is done. At 1, any line 1 pixel away from another is merged... and so
    # on as you increase this value. You may wish to merge lines as line segment
    # detection produces a lot of line segment results.

    # `max_theta_diff` controls the maximum amount of rotation difference between
    # any two lines about to be merged. The default setting allows for 15 degrees.
    line = img.get_regression([(240, 255) if BINARY_VISIBLE else THRESHOLD], area_threshold = 0, pixels_threshold = 20, robust = True)
    if (line): img.draw_line(line.line(), color = 127)
    if line:
        #print(line.line())
        uart.write(("/Line/run %d %d %d %d\r\n" % line.line()).encode())
        time.sleep_ms(20)
    else :
        uart.write(("/stop/run \r\n").encode())
        time.sleep_ms(20)
