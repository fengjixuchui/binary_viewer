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

#ifndef _OVERALL_VIEW_H_
#define _OVERALL_VIEW_H_

#include <QLabel>
#include <QImage>
#include <QPixmap>

class OverallView : public QLabel {
Q_OBJECT
public:
    explicit OverallView(QWidget *p = nullptr);

    ~OverallView() override = default;

public slots:

    void setImage(QImage &img);

    void set_data(const unsigned char *bin, long len, bool reset_selection = true);

    void enableSelection(bool);

protected slots:

protected:
    QImage img_;
    QPixmap pix_;

    void paintEvent(QPaintEvent *) override;

    void resizeEvent(QResizeEvent *e) override;

    void mousePressEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

    void update_pix();

    float m1_, m2_;
    int px_, py_;
    enum {
        none, m1_moving, m2_moving, m12_moving
    } s_;
    bool allow_selection_;

    bool use_byte_classes_;
    bool use_hilbert_curve_;

    const unsigned char *dat_;
    long len_;

signals:

    void rangeSelected(float, float);
};

#endif
