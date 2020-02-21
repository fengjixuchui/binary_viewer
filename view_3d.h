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

#ifndef _VIEW_3D_H_
#define _VIEW_3D_H_

#include <QGLWidget>

class QSpinBox;

class QComboBox;

class QCheckBox;

class View3D : public QGLWidget {
Q_OBJECT
public:
    explicit View3D(QWidget *p = nullptr);

    ~View3D() override;

public slots:

    void setData(const unsigned char *dat, long n);

    void parameters_changed();

protected slots:

    void regen_histo();

protected:
    void initializeGL() override;

    void resizeGL(int w, int h) override;

    void paintGL() override;

    void mousePressEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

    typedef enum {
        none, u8, u12, u16, u32, u64, f32, f64
    } dtype_t;

    int *generate_histo(const unsigned char *, long int, dtype_t);

    QSpinBox *thresh_, *scale_;
    QComboBox *type_;
    QCheckBox *overlap_;
    int *hist_;
    const unsigned char *dat_;
    long dat_n_;
    bool spinning_;
};

#endif
