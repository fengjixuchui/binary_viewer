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

#include <vector>
#include <algorithm>

#include <QtGui>
#include <QGridLayout>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>

#include "dot_plot.h"

using std::max;
using std::min;
using std::vector;
using std::random_shuffle;
using std::pair;
using std::make_pair;

DotPlot::DotPlot(QWidget *p)
        : QLabel(p),
          dat_(nullptr), dat_n_(0),
          mat_(nullptr), mat_max_n_(0), mat_n_(0),
          pts_i_(0) {
    {
        auto layout = new QGridLayout(this);
        int r = 0;

        {
            auto l = new QLabel("Offset1 (B)");
            l->setFixedSize(l->sizeHint());
            layout->addWidget(l, r, 0);
        }
        {
            auto sb = new QSpinBox;
            sb->setFixedSize(sb->sizeHint());
            sb->setFixedWidth(sb->width() * 1.5);
            sb->setRange(0, 100000);
            sb->setValue(0);
            offset1_ = sb;
            layout->addWidget(sb, r, 1);
        }
        r++;

        {
            auto l = new QLabel("Offset2 (B)");
            l->setFixedSize(l->sizeHint());
            layout->addWidget(l, r, 0);
        }
        {
            auto sb = new QSpinBox;
            sb->setFixedSize(sb->sizeHint());
            sb->setFixedWidth(sb->width() * 1.5);
            sb->setRange(0, 100000);
            sb->setValue(0);
            offset2_ = sb;
            layout->addWidget(sb, r, 1);
        }
        r++;

        {
            auto l = new QLabel("Width");
            l->setFixedSize(l->sizeHint());
            layout->addWidget(l, r, 0);
        }
        {
            auto sb = new QSpinBox;
            sb->setFixedSize(sb->sizeHint());
            sb->setFixedWidth(sb->width() * 1.5);
            sb->setRange(1, 100000);
            sb->setValue(10000);
            width_ = sb;
            layout->addWidget(sb, r, 1);
        }
        r++;

        {
            auto l = new QLabel("Max Samples");
            l->setFixedSize(l->sizeHint());
            layout->addWidget(l, r, 0);
        }
        {
            auto sb = new QSpinBox;
            sb->setFixedSize(sb->sizeHint());
            sb->setFixedWidth(sb->width() * 1.5);
            sb->setRange(1, 100000);
            sb->setValue(10);
            max_samples_ = sb;
            layout->addWidget(sb, r, 1);
        }
        r++;

        {
            auto pb = new QPushButton("Resample");
            pb->setFixedSize(pb->sizeHint());
            layout->addWidget(pb, r, 1);
            QObject::connect(pb, SIGNAL(clicked()), this, SLOT(parameters_changed()));
        }
        r++;

        layout->setColumnStretch(2, 1);
        layout->setRowStretch(r, 1);

        QObject::connect(offset1_, SIGNAL(valueChanged(int)), this, SLOT(parameters_changed()));
        QObject::connect(offset2_, SIGNAL(valueChanged(int)), this, SLOT(parameters_changed()));
        QObject::connect(width_, SIGNAL(valueChanged(int)), this, SLOT(parameters_changed()));
        QObject::connect(max_samples_, SIGNAL(valueChanged(int)), this, SLOT(parameters_changed()));
    }
}

DotPlot::~DotPlot() {
    delete[] mat_;
}

void DotPlot::setImage(QImage &img) {
    img_ = img;

    update_pix();

    update();
}

void DotPlot::paintEvent(QPaintEvent *e) {
    QLabel::paintEvent(e);

    QPainter p(this);
    {
        // a border around the image helps to see the border of a dark image
        p.setPen(Qt::darkGray);
        p.drawRect(0, 0, width() - 1, height() - 1);
    }
}

void DotPlot::resizeEvent(QResizeEvent *e) {
    QLabel::resizeEvent(e);

    int tmp = min(width(), height());
    if (tmp != mat_max_n_) {
        delete[] mat_;
        mat_max_n_ = tmp;
        mat_n_ = 0;
        mat_ = new int[mat_max_n_ * mat_max_n_];
    }

    parameters_changed();
}

void DotPlot::update_pix() {
    if (img_.isNull()) return;

    int vw = width() - 4;
    int vh = height() - 4; // TODO BUG: With QDarkStyle, without the subtraction, the height or width of the application grows without bounds.
    pix_ = QPixmap::fromImage(img_).scaled(vw, vh); //, Qt::KeepAspectRatio);
    setPixmap(pix_);
}


void DotPlot::setData(const unsigned char *dat, long n) {
    dat_ = dat;
    dat_n_ = n;

    offset1_->setRange(0, dat_n_);
    offset2_->setRange(0, dat_n_);
    width_->setRange(1, dat_n_);

    width_->setValue(dat_n_);

    // parameters_changed() triggered by the previous setValue() call.
    // parameters_changed();
}

void DotPlot::parameters_changed() {
    puts("called");

    long mdw = min(dat_n_, (long) width_->value());
    int bs = int(mdw / mat_max_n_) + ((mdw % mat_max_n_) > 0 ? 1 : 0);
    mat_n_ = 0;

    if (dat_n_ > 0) {
        mat_n_ = min(int(mdw / bs), mat_max_n_);
        printf("Setting max to %d\n", bs);
        max_samples_->setMaximum(bs);
    }

    printf("dat_n_%d mdw:%d mat_max_n_:%d bs:%d mat_n_:%d bs * mat_n_:%d\n", dat_n_, mdw, mat_max_n_, bs, mat_n_, bs * mat_n_);

    memset(mat_, 0, sizeof(mat_[0]) * mat_max_n_ * mat_max_n_);

    pts_.clear();
    pts_.reserve(mat_n_ * mat_n_);
#if 1
    for (int i = 0; i < mat_n_; i++) {
        for (int j = i; j < mat_n_; j++) {
            pts_.emplace_back(make_pair(i, j));
        }
    }
#else
    for (int i = 0; i < mat_n_; i++) {
        pts_.emplace_back(make_pair(i, mat_n_-i-1));
//        pts_.emplace_back(make_pair(i, i));
    }
#endif
    random_shuffle(pts_.begin(), pts_.end());

    {
//        float sf = 1.f;
//        if (mwh < mdw) {
//            sf = mwh / float(mdw);
//        }
//
//        int xyn = 1 / sf + 1;

//        printf("min(width(), height()): %d mwh:%d mdw:%d dat_n:%d sf:%f xyn:%d", min(width(), height()), mwh, mdw, dat_n_, sf, xyn);


//        printf("pts_.size(): %d sf: %f\n", pts_.size(), sf);
        // pts_i_ is decremented in advance_mat()
        pts_i_ = pts_.size();
        int ii = 0;
        // Precompute some random values for sampling
        std::vector<pair<int, int> > rand;
        while (pts_i_ > 0) {
            if ((ii++ % 100) == 0) {
                {
                    int n = min(max_samples_->value(), bs);
//            n = n * n;
                    rand.clear();
                    rand.reserve(n);
                    printf("Generating %d points in range [0, %d-1] along the diagonal\n", n, bs);
                    for (int tt = 0; tt < n; tt++) {
                        int a = random() % bs;
                        rand.emplace_back(make_pair(a, a));
                    }
                    int n2 = min(max_samples_->value(), bs * bs - bs);
                    printf("Generating %d points in range [0, %d-1] off the diagonal\n", n2, bs);
                    for (int tt = 0; tt < n2;) {
                        int a = random() % bs;
                        int b = random() % bs;
                        if (a == b) continue;
                        rand.emplace_back(make_pair(a, b));
                        tt++;
                    }
//            for (int tt = 0; tt < n; tt++) {
//                rand.emplace_back(make_pair(random() % bs, random() % bs));
//            }
                }
            }


//            random_shuffle(rand.begin(), rand.end());
            advance_mat(bs, rand);
        }
    }
    regen_image();
}

void DotPlot::advance_mat(int bs, const vector<pair<int, int> > &rand) {
    if (pts_i_ == 0 || pts_.empty()) return;

    pts_i_--;
    pair<int, int> pt = pts_[pts_i_];

    int x = pt.first;
    int y = pt.second;

    int xo = x * bs;
    int yo = y * bs;

    if (true) {
//        for (int tt = 0; tt < bs; tt++) {
//            int i = xo + tt;
//            int j = yo + tt;
//
//            if (dat_[i] == dat_[j]) {
//                int ii = y * mat_n_ + x;
//                int jj = x * mat_n_ + y;
//                if (0 <= ii && ii < mat_n_ * mat_n_) mat_[ii]++;
//                if (0 <= jj && jj < mat_n_ * mat_n_) mat_[jj]++;
//            }
////            printf("%d %d %d %d %d %d %d %d\n", xo, yo, tt, rand[tt], i, j, dat_[i], dat_[j]);
//        }
        for (int tt = 0; tt < rand.size(); tt++) {
            int i = xo + rand[tt].first;
            int j = yo + rand[tt].second;
//            int i = xo + (random() % bs);
//            int j = yo + (random() % bs);

            if (dat_[i] == dat_[j]) {
                int ii = y * mat_n_ + x;
                int jj = x * mat_n_ + y;
                if (0 <= ii && ii < mat_n_ * mat_n_) mat_[ii]++;
                if (0 <= jj && jj < mat_n_ * mat_n_) mat_[jj]++;
            }
//            printf("%d %d %d %d %d %d %d %d\n", xo, yo, tt, rand[tt], i, j, dat_[i], dat_[j]);
        }
    } else {
        for (int tt1 = 0; tt1 < bs; tt1++) {
            for (int tt2 = 0; tt2 < bs; tt2++) {
                int i = xo + tt1;
                int j = yo + tt2;

                if (dat_[i] == dat_[j]) {
                    int ii = y * mat_n_ + x;
                    int jj = x * mat_n_ + y;
                    if (0 <= ii && ii < mat_n_ * mat_n_) mat_[ii]++;
//                    else abort();
                    if (0 <= jj && jj < mat_n_ * mat_n_) mat_[jj]++;
//                    else abort();
                }
                printf("%d %d %d %d %d %d %d %d\n", xo, yo, tt1, tt2, i, j, dat_[i], dat_[j]);
            }
        }
    }
}

void DotPlot::regen_image() {
    // Find the maximum value, ignoring the diagonal.
    // Could stop the search once m = max_samples_->value()
    int m = 0;
    for (int j = 0; j < mat_n_; j++) {
        for (int i = 0; i < j; i++) {
            int k = j * mat_n_ + i;
            if (m < mat_[k]) m = mat_[k];
        }
        for (int i = j + 1; i < mat_n_; i++) {
            int k = j * mat_n_ + i;
            if (m < mat_[k]) m = mat_[k];
        }
    }

    printf("max(1): %d\n", m);
    if (true) {
        // Brighten image
        m = max(1, int(m * .75));
        printf("max(2): %d\n", m);
    }

    QImage img(mat_n_, mat_n_, QImage::Format_RGB32);
    img.fill(0);
    auto p = (unsigned int *) img.bits();
    for (int i = 0; i < mat_n_ * mat_n_; i++) {
        int c = min(255, int(mat_[i] / float(m) * 255. + .5));
        unsigned char r = c;
        unsigned char g = c;
        unsigned char b = c;
        unsigned int v = 0xff000000 | (r << 16) | (g << 8) | (b << 0);
        *p++ = v;
    }

    img.save("a.png");

//    long mdw = min(dat_n_, (long) width_->value());
//    int mwh = min(width(), height());
//    if (mwh > mdw) mwh = mdw;

    setImage(img);
}
