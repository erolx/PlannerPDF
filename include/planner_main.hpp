#ifndef PLANNER_MAIN_HPP
#define PLANNER_MAIN_HPP
// The MIT License (MIT)
//
// Copyright (c) 2015, 2016, 2017 Howard Hinnant
// Copyright (c) 2016 Adrian Colomitchi
// Copyright (c) 2017 Florian Dang
// Copyright (c) 2017 Paul Thompson
// Copyright (c) 2018, 2019 Tomasz Kamiński
// Copyright (c) 2019 Jiangang Zhuang
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Our apologies.  When the previous paragraph was written, lowercase had not
// yet been invented (that would involve another several millennia of
// evolution). We did not mean to shout.
#include "planner_year.hpp"

/*!
 * @brief
 * The Main Planner page class
 */
class PlannerMain : public PlannerBase {
  date::year_month_day _base_date;
  std::string _filename;
  std::uint64_t _num_years;
  std::vector<std::shared_ptr<PlannerBase>> _years;
  short _first_day_of_week;
  HPDF_Doc _pdf;

public:
  PlannerMain()
      : _base_date((date::year)2021, (date::month)1, (date::day)1),
        _num_years(10), _filename("test.pdf") {
    _page_title = "Planner";
    _note_section_percentage = 0.5;
  }

  PlannerMain(short year,
              std::string filename,
              short num_years,
              HPDF_REAL height,
              HPDF_REAL width,
              HPDF_REAL margin,
              short first_day_of_week,
              bool is_left_handed,
              bool is_portrait,
              bool time_in_margin,
              int time_gap_lines,
              int time_start
              )
      : _base_date((date::year)year, (date::month)1, (date::day)1),
        _filename(filename), _num_years(num_years)  {
    _page_title = "  Planner  ";
    _page_height = height;
    _page_width = width;
    _margin_width = margin;
    _margin_left = _margin_width;
    _margin_right = _page_width - _margin_width;
    _first_day_of_week = first_day_of_week;
    _is_left_handed = is_left_handed;
    _is_portrait = is_portrait;
    _note_section_percentage = 0.5;
    _time_in_margin = time_in_margin;
    _time_gap_lines = time_gap_lines;
    _time_start = time_start;
  }

  static void
  err_cb(HPDF_STATUS error_no, HPDF_STATUS detail_no, void* user_data) {
    std::cout << "[ERR] : PDF operation error : Received error : " << std::hex
              << error_no << " with detail : " << detail_no << std::endl;

    throw std::exception();
  }

  void CreateDocument() {
    _pdf = HPDF_New(this->err_cb, NULL);
    if (NULL == _pdf) {
      std::cout << "[ERR] Failed to create PDF object" << std::endl;
      throw std::exception();
    }
  }

  void CreateYearsSection(HPDF_Doc& doc) {
    std::string year_title_string = "Years";
    HPDF_REAL notes_divider_x = _page_width * _note_section_percentage;
    HPDF_REAL years_section_text_x;
    HPDF_REAL section_x_start;
    HPDF_REAL section_y_start;
    HPDF_REAL section_x_stop;
    HPDF_REAL section_y_stop;

    if (true == _is_left_handed) {
      section_x_start = 0;
      section_y_start = _page_title_font_size * 2;
      section_x_stop = notes_divider_x;
      section_y_stop = _page_height;
      years_section_text_x = GetCenteredTextXPosition(
          _page, year_title_string, section_x_start, section_x_stop);
    } else {
      section_x_start = notes_divider_x;
      section_y_start = _page_title_font_size * 2;
      section_x_stop = _page_width;
      section_y_stop = _page_height;
      years_section_text_x = GetCenteredTextXPosition(
          _page, year_title_string, section_x_start, section_x_stop);
    }

    HPDF_Page_BeginText(_page);
    HPDF_Page_MoveTextPos(_page,
                          years_section_text_x,
                          _page_height - (_page_title_font_size * 2) -
                              _note_title_font_size - 10);
    HPDF_Page_ShowText(_page, year_title_string.c_str());
    HPDF_Page_EndText(_page);

    CreateGrid(doc,
               _page,
               section_x_start + 20,
               section_y_start + (_note_title_font_size * 2),
               section_x_stop - 20,
               section_y_stop,
               _years.size(),
               1,
               _years,
               true,
               0,
               false,
               PlannerTypes_Main,
               PlannerTypes_Year,
               _page_height,
               10,
               true);
  }

  void BuildYears() {
    for (auto year : _years) {
      std::static_pointer_cast<PlannerYear>(year)->Build(_pdf);
    }
  }

  void CreateNavigation() {
    for (auto year : _years) {
      std::static_pointer_cast<PlannerYear>(year)->CreateNavigation(_pdf);
    }
  }

  void Build() {
    CreatePage(_pdf, _page_height, _page_width);
    /* Add _num_years of year objects and call their build functions */
    for (size_t loop_index = 0; loop_index < _num_years; loop_index++) {
      date::year next_year = _base_date.year() + (date::years)loop_index;
      _years.push_back(
          std::make_shared<PlannerYear>(PlannerYear(next_year,
                                                    shared_from_this(),
                                                    _page_height,
                                                    _page_width,
                                                    _margin_width,
                                                    _first_day_of_week,
                                                    _is_left_handed,
                                                    _is_portrait,
                                                    _time_in_margin,
                                                    _time_gap_lines,
                                                    _time_start)));
      if (loop_index != 0) {
        _years.back()->SetLeft(_years[loop_index - 1]);
        _years[loop_index - 1]->SetRight(_years.back());
      }
    }
    CreateTitle();
    BuildYears();
    CreateNavigation();
    CreateNotesSection(false);
    CreateYearsSection(_pdf);
  }

  void FinishDocument() {
    HPDF_SaveToFile(_pdf, _filename.c_str());
    HPDF_Free(_pdf);
  }
};
#endif // PLANNER_MAIN_HPP
