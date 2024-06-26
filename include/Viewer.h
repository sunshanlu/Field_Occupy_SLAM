#pragma once

#include <atomic>
#include <mutex>

#include "Common.h"
#include "Frame.h"
#include "SubMap.h"

namespace fos {

class Viewer {
public:
    using Ptr = std::shared_ptr<Viewer>;
    using SubMaps = std::vector<SubMap::Ptr>;
    using C2ImgFunc = std::function<cv::Point2i(const Vec2 &pc)>;

    Viewer(Options::Ptr options)
        : resq_stop_(false)
        , options_(std::move(options))
        , gminx_(1e6)
        , gminy_(1e6)
        , gmaxx_(-1e6)
        , gmaxy_(-1e6) {
        /// 允许保持窗口比例的缩放
        cv::namedWindow("SubMap", cv::WINDOW_KEEPRATIO);
        cv::namedWindow("Map", cv::WINDOW_KEEPRATIO);
        cv::resizeWindow("SubMap", cv::Size(1000, 500));
        cv::resizeWindow("Map", cv::Size(1000, 1000));
    }

    ~Viewer() { cv::destroyAllWindows(); }

    /// 设置子地图
    void SetSubMap(SubMap::Ptr submap, SubMaps submaps) {
        std::lock_guard<std::mutex> lock(sub_mutex_);
        submap_ = std::move(submap);
        all_submaps_ = std::move(submaps);
        submap_update_ = true;
    }

    /// 设置当前雷达帧
    void SetFrame(Frame::Ptr frame) {
        std::lock_guard<std::mutex> lock(frame_mutex_);
        frame_ = std::move(frame);
    }

    /// 请求可视化线程停止
    void RequestStop() { resq_stop_.store(true); }

    void Run();

private:
    /// 绘制子地图
    cv::Mat DrawSubMap(cv::Mat field, cv::Mat occupy, const SubMap::Ptr &submap);

    /// 将雷达帧绘制到子地图上
    void DrawFrame(cv::Mat &SubMapImg, const SubMap::Ptr &submap, const Frame::Ptr &frame);

    /// 绘制Robot信息
    void DrawRobotSub(cv::Mat &SubMapImg, const SubMap::Ptr &submap, const Frame::Ptr &frame);

    /// 绘制Robot信息
    void DrawRobot(cv::Mat &Map, const SE2 &rpose, const C2ImgFunc &c2img);

    /// 绘制全局地图
    cv::Mat DrawGlobalMap(int max_size, const SubMaps &submaps, const Frame::Ptr &frame, bool update);

    /// 绘制轨迹
    void DrawTraject(cv::Mat &Map, const std::vector<SE2> &poses, const C2ImgFunc &c2img);

    /// 绘制子地图坐标系
    void DrawSubAxis(cv::Mat &Map, const SubMaps &submaps, const C2ImgFunc &c2img);

    SubMap::Ptr submap_;          ///< 维护的子地图
    Frame::Ptr frame_;            ///< 维护的当前帧
    std::mutex sub_mutex_;        ///< 子地图互斥锁
    std::mutex frame_mutex_;      ///< 当前帧互斥锁
    std::atomic<bool> resq_stop_; ///< 是否有外部命令停止
    SubMaps all_submaps_;         ///< 所有子地图
    Options::Ptr options_;        ///< 系统配置指针，用于获取robot信息
    bool submap_update_;          ///< 子地图更新标志

    float gminx_, gminy_, gmaxx_, gmaxy_; ///< 全局地图的尺寸
    float resolution_, cx_, cy_;          ///< 全局地图投影参数
};

} // namespace fos