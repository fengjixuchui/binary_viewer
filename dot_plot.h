/*
 * Copyright (c) 2015, 2017, 2020 Kent A. Vander Velden, kent.vandervelden@gmail.com
 *
 * This file is part of BinVis.
 *
 *     BinVis is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 *
 *     BinVis is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with BinVis.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef _DOTPLOT_H_
#define _DOTPLOT_H_

#include <QLabel>
#include <QImage>
#include <QPixmap>

class QSpinBox;

class DotPlot : public QLabel {
Q_OBJECT
public:
    explicit DotPlot(QWidget *p = nullptr);

    ~DotPlot() override;

public slots:

    void setData(const unsigned char *dat, long n);

    void parameters_changed();

protected slots:

    void setImage(QImage &img);

    void advance_mat();

    void regen_image();

protected:
    QImage img_;
    QPixmap pix_;

    void paintEvent(QPaintEvent *) override;

    void resizeEvent(QResizeEvent *e) override;

    void update_pix();

    typedef enum {
        none, rgb8, rgb16, rgba8, rgba16, bgr8, bgr16, bgra8, bgra16, grey8, grey16
    } dtype_t;

    QSpinBox *offset1_, *offset2_, *width_, *max_samples_;
    const unsigned char *dat_;
    long dat_n_;
};

#endif
