#define GL_GLEXT_PROTOTYPES
#include <GL/glut.h>
#include<unistd.h>
#include <iostream>
#include <sys/time.h>
#include <cstdlib>
#include <cstdio>
#include <sstream>
#include <deque>
#include "simple_render.hh"
#include "gfmesh.hh"
#include "common_def.hh"
#include "vertexpq.hh"

void SimpleRender::keyboard(unsigned char key, int, int)
{
    std::ofstream ofs;
    switch (key)
    {
    case 27:
        sleep(1);
        exit(0);
        break;
    }
}

void SimpleRender::check_visibility()
{
    glDisable(GL_LIGHTING);
    glDisable(GL_DITHER);
    unsigned char color_r = 0;
    unsigned char color_g = 0;
    unsigned char color_b = 1;
    glBegin(GL_TRIANGLES);
    for (size_t i=0; i<gfmesh_->face_number(); i++)
    {
        VertexIndex v1 = gfmesh_->vertex1_in_face(i);
        VertexIndex v2 = gfmesh_->vertex2_in_face(i);
        VertexIndex v3 = gfmesh_->vertex3_in_face(i);
        glColor3ub(color_r, color_g, color_b);
        glArrayElement(v1);
        glArrayElement(v2);
        glArrayElement(v3);
        if (color_b == 255)
        {
            color_b = 0;
            if (color_g == 255)
            {
                color_g = 0;
                if (color_r == 255)
                {
                    std::cerr<<"overflow!"<<std::endl;
                }
                else
                {
                    color_r ++;
                }
            }
            else
            {
                color_g ++;
            }
        }
        else
        {
            color_b++;
        }
    }
    glEnd();
    glReadBuffer(GL_BACK);
    glReadPixels(0,0,width_, height_, GL_RGB, GL_UNSIGNED_BYTE, pixels_);
    pq_->update(pixels_, width_*height_*3, width_);
    glEnable(GL_LIGHTING);
    glEnable(GL_DITHER);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    to_check_visibility_ = false;
}

void SimpleRender::draw_surface_with_arrays()
{
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_DOUBLE, 0, gfmesh_->vertex_array());
    
    if (pq_ != 0 && to_check_visibility_)
    {
        check_visibility();
    }
    
    glDisableClientState(GL_NORMAL_ARRAY);
    glBegin(GL_TRIANGLES);
    for (size_t i=0; i< gfmesh_->face_number(); i++)
    {
        if (gfmesh_->face_is_visible(i))
        {
            glNormal3d(gfmesh_->face_normal_array()[3*i], gfmesh_->face_normal_array()[3*i+1], gfmesh_->face_normal_array()[3*i+2]);
            VertexIndex v1 = gfmesh_->vertex1_in_face(i);
            VertexIndex v2 = gfmesh_->vertex2_in_face(i);
            VertexIndex v3 = gfmesh_->vertex3_in_face(i);
            glArrayElement(v1);
            glArrayElement(v2);
            glArrayElement(v3);
        }
    }
    glEnd();
    if (to_output_)
    {
        glReadPixels(0,0,width_, height_, GL_RGB, GL_UNSIGNED_BYTE, pixels_);
        if (original_pixels_)
        {
            check_psnr(width_*height_, max_value_, original_pixels_, pixels_, &psnr_, &error_count_);
        }
        to_output_ = false;
    }
    rendered_ = true;
}

void SimpleRender::outputImage(std::ostream& os)
{

    os<<"P5 "<<width_<<" "<<height_<<" 255"<<std::endl;
    unsigned char* ptr = pixels_;
    for (size_t i = 0; i< size_t(width_*height_); i++)
    {
            unsigned char r = *(ptr++);
            unsigned char g = *(ptr++);
            unsigned char b = *(ptr++);
            unsigned char y = (unsigned char)(0.299*r + 0.587*g + 0.114*b);
            os.write((char*)&y, 1);
    }
}

void SimpleRender::idle()
{
    if (pq_ == 0)
    {
        output_best_image();
    }
    else
    {
        do_main();
    }
}

void SimpleRender::output_best_image(void)
{
    //std::cerr << "in output" << std::endl;
    if (rendered_)
    {
        std::string output_name = prefix_ + "_final_image.pgm";
        std::ofstream ofs(output_name.c_str());
        outputImage(ofs);
        ofs.close();
        exit(0);
    }
}

VertexID SimpleRender::next_to_be_split()
{
    VertexID id = gfmesh_->to_be_split();
    if (id == 0) // No pending splits
    {
        id = pq_->pop();
    }
    return id;
}

void SimpleRender::push_buffer(int n)
{
    int i = 0;
    while(i < n)
    {
        VertexID id = next_to_be_split();
        //if (id == 0) break;
        if (id == 0)
        {
            if (valid_splits_ > 0)
            {
                buffer_.push_back(id);
                i++;
            }
            else
            {
                exit(0);
            }
        }
        else if (batch_size_ <= 1 || requested_.find(id) == requested_.end() || !requested_[id])
        {
            buffer_.push_back(id);
            //std::cerr << "pushed " << id << std::endl;
            requested_[id] = true;
            valid_splits_ ++;
            i++;
        }
    }
    //std::cerr<<"pushed. Now valid: "<<valid_splits_<<std::endl;
}

void SimpleRender::do_main()
{
    int image_step  = 1000;
    static int last_image = 0;
    static bool first = true;
    if (rendered_)
    {
        //std::cout << count_ << " " << id_ << " " << bs_size_ << " ";
        //std::cout << total_bs_size_ << " " << psnr_ << " " << error_count_ << "\n";
        if (count_ / image_step >= last_image)
        {
            std::string output_name;
            std::stringstream sstr(output_name);
            sstr << prefix_ << count_ << ".pgm";
            std::ofstream ofs(sstr.str().c_str());
            outputImage(ofs);
            ofs.close();
            last_image ++;
        }
        to_output_ = true;

        if (first)
        {
            //initial step
            push_buffer(initial_size_);
            /*for (int i = 0; i < batch_size_; i++)
            {
                if (buffer_.empty())
                {
                    exit(0);
                }
                id_ = buffer_.front();
                buffer_.pop_front();
                if (id_ == 0)
                {
                    bs_size_ = 0;
                }
                else
                {
                    size_t pos = 0;
                    BitString bs = split_map_[id_];
                    gfmesh_->decode(id_, bs, &pos);
                    bs_size_ = bs.size();
                    total_bs_size_ += bs_size_;
                    valid_splits_ --;
                }
                count_ ++;
                std::cout << count_ << " " << id_ << " " << bs_size_ << " ";
                std::cout << total_bs_size_  / 8 << " " << psnr_ << " " << error_count_ << "\n";
            }*/
            first = false;
        }
        else
        {
            push_buffer(batch_size_);
            for (int i = 0; i < batch_size_; i++)
            {
                if (count_ > total_count_) 
                {
                    exit(0);
                }
                if (buffer_.empty())
                {
                    exit(0);
                }
                id_ = buffer_.front();
                //std::cerr << "id " << id_ << std::endl;
                buffer_.pop_front();
                if (id_ == 0) 
                {
                    bs_size_ = 0;
                }
                else
                {
                    size_t pos = 0;
                    if (split_map_.find(id_) == split_map_.end())
                    {
                        std::cerr << "invalid id to split: " << id_ << std::endl;
                        exit(1);
                    }
                    BitString bs = split_map_[id_];
                    gfmesh_->decode(id_, bs, &pos);
                    bs_size_ = bs.size();
                    total_bs_size_ += bs_size_;
                    valid_splits_ --;
                }
                count_ ++;
                std::cout << count_ << " " << id_ << " " << bs_size_ << " ";
                std::cout << total_bs_size_ / 8 << " " << psnr_ << " " << error_count_ << "\n";
            }
            //std::cerr <<"valid "<<valid_splits_<<std::endl;
            //std::cerr <<"one round "<<count_<<std::endl;
        }
        gfmesh_->update();
        
        rendered_ = false;
        if (count_ / update_period_ * update_period_ == count_)
        {
            //std::cerr << count_ << std::endl;
            to_check_visibility_ = true;
        }
        glutPostRedisplay();
    }
}

SimpleRender::SimpleRender(int argc, char *argv[], const char *name, Vdmesh *gfmesh, std::map<VertexID, BitString>& split_map, const Center& center, VertexPQ *pq, std::string prefix, int initial_size, int batch_size, int update_period, int total_count) 
        :BaseRender(argc, argv, name, false), gfmesh_(gfmesh), split_map_(split_map), pq_(pq), prefix_(prefix), initial_size_(initial_size), batch_size_(batch_size), update_period_(update_period), total_count_(total_count), \
         count_(0), id_(0), bs_size_(0), total_bs_size_(0), psnr_(0), error_count_(0), \
         original_pixels_(0), max_value_(255), \
        to_output_(true), to_check_visibility_(false), rendered_(false), valid_splits_(0)
{
    if (pq_ != 0)
    {
        to_check_visibility_ = true;
    }
    
    set_center(center);
    
    framerate_ = 50;
    initGlut(argc, argv);
    
    std::cerr<<view_x_<<" "<<view_y_<<" "<<view_z_<<" "<<std::endl;
    std::cerr<<bounding_length_<<std::endl;
}

void SimpleRender::set_original_image(const std::string& pgm_file)
{
    FILE* fpgm = fopen(pgm_file.c_str(), "rb");
    assert(fpgm);
    size_t len = 1024 * 768;
    original_pixels_ = new unsigned char[1024 * 768];
    read_pgm(fpgm, original_pixels_, &len, &max_value_);
    //std::cout << "max" << max_value_ << std::endl;
}
