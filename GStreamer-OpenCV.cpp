// GStreamer-OpenCV.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <opencv2/opencv.hpp>
#include "cameraman.h"

#include <iostream>
#include <sstream>

#include <gst/gst.h>

/* bar-menu.c
Copyright (c) 2011, Frank Cox <theatre@melvilletheatre.com>
December 7, 2011
All rights reserved.
Redistribution and use in source and binary forms, with or without modification, are permitted
provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
THIS SOFTWARE IS PROVIDED BY FRANK COX ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL FRANK COX BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
OF SUCH DAMAGE.
 */

#include <curses.h>

static int barmenu(const std::vector<std::string>& array, const int row, const int col, const int width, int menulength, int selection = 0)
{
    int counter, offset = 0, ky = 0;
    char formatstring[7];
    curs_set(0);

    const int arraylength = array.size();

    if (arraylength < menulength)
        menulength = arraylength;

    if (selection > menulength)
        offset = selection - menulength + 1;

    snprintf(formatstring, sizeof(formatstring) / sizeof(formatstring[0]), "%%-%ds", width); // remove - sign to right-justify the menu items

    while (ky != 27)
    {
        for (counter = 0; counter < menulength; counter++)
        {
            if (counter + offset == selection)
                attron(A_REVERSE);
            mvprintw(row + counter, col, formatstring, array[counter + offset].c_str());
            attroff(A_REVERSE);
        }

        ky = getch();

        switch (ky)
        {
        case KEY_UP:
            if (selection)
            {
                selection--;
                if (selection < offset)
                    offset--;
            }
            break;
        case KEY_DOWN:
            if (selection < arraylength - 1)
            {
                selection++;
                if (selection > offset + menulength - 1)
                    offset++;
            }
            break;
        case KEY_HOME:
            selection = 0;
            offset = 0;
            break;
        case KEY_END:
            selection = arraylength - 1;
            offset = arraylength - menulength;
            break;
        case KEY_PPAGE:
            selection -= menulength;
            if (selection < 0)
                selection = 0;
            offset -= menulength;
            if (offset < 0)
                offset = 0;
            break;
        case KEY_NPAGE:
            selection += menulength;
            if (selection > arraylength - 1)
                selection = arraylength - 1;
            offset += menulength;
            if (offset > arraylength - menulength)
                offset = arraylength - menulength;
            break;
        case 10: //enter
            return selection;
            break;
        case KEY_F(1): // function key 1
            return -1;
        case 27: //esc
                // esc twice to get out, otherwise eat the chars that don't work
                //from home or end on the keypad
            ky = getch();
            if (ky == 27)
            {
                curs_set(0);
                mvaddstr(9, 77, "   ");
                return -1;
            }
            else
                if (ky == '[')
                {
                    getch();
                    getch();
                }
                else
                    ungetch(ky);
        }
    }
    return -1;
}

int main(int argc, char *argv[])
{
    gst_init(&argc, &argv);

    const auto cameraDescriptions = getCameraDescriptions();
    if (cameraDescriptions.empty())
    {
        std::cerr << "No cameras" << std::endl;
        return -1;
    }


    initscr();
    noecho();
    keypad(stdscr, TRUE);

    std::vector<std::string> items;
    for (auto& desc : cameraDescriptions)
    { 
        items.push_back(desc.description);
    }

    const int row = 1;
    const int col = 5;
    const int menuwidth = 70;
    const int menulength = 20;

    mvprintw(0, 0, "Please select a camera:");

    int selection = barmenu(items, row, col, menuwidth, menulength);
    if (selection == -1)
    {
        return 0;
    }

    const auto& desc = cameraDescriptions[selection];

    items.clear();
    for (auto& mode : desc.modes)
    {
        items.push_back(mode.getDescr());
    }

    refresh();

    mvprintw(0, 0, "Please select a camera mode:");

    selection = barmenu(items, row, col, menuwidth, menulength);
    if (selection == -1)
    {
        return 0;
    }

    const auto& mode = desc.modes[selection];

    std::ostringstream s;

    s << desc.launchLine
        << " ! video/x-raw,format=" << mode.format << ",width=" << mode.w << ",height=" << mode.h << ",framerate=" << mode.den << "/" << mode.num
        << " ! videoconvert ! appsink";

    // ksvideosrc ! video/x-raw,format=I420,width=640,height=480,framerate=30/1
    //VideoCapture cap("ksvideosrc ! video/x-raw,format=I420,width=640,height=480,framerate=30/1 ! videoconvert ! appsink",
    cv::VideoCapture cap(s.str(), //"ksvideosrc ! video/x-raw ! videoconvert ! appsink",
        cv::CAP_GSTREAMER);

    if (!cap.isOpened()) {
        std::cerr << "VideoCapture not opened" << std::endl;
        return -1;
    }

    while (true) {

        cv::Mat frame;

        cap.read(frame);

        cv::imshow("receiver", frame);

        if (cv::waitKey(1) == 27) {
            break;
        }
    }

    return 0;
}
