# 多线程 Tile 光栅化设计

日期：2026-08-23
状态：已确认（用户授权连续执行模式）

## 方案

Tile-based 并行化，最小侵入复用既有 Rasterizer：

- `Render/TileRenderer.hpp` 新组件：
```cpp
class TileRenderer{
public:
    explicit TileRenderer(FrameBuffer &fb);
    void drawTextured(const std::vector<ScreenTriangle> &tris,
                      const Texture &tex, const ShadingContext *ctx,
                      TextureFilter filter, TextureWrap wrap,
                      unsigned threads = std::thread::hardware_concurrency());
};
```
- 屏幕切 64×64 tile；按三角形包围盒与 tile 相交入桶（保序）
- 每 worker 认领 tile 区间，桶内三角形按全局提交序逐个调用既有 `Rasterizer::drawTriangleTextured`
- **正确性论证**：任一像素被处理的三角形序列与串行完全一致 → 深度/混合语义逐位等价；不同 tile 触碰不相交像素集合 → 无数据竞争
- 主线程 join 全部 worker 后返回

## 测试

1. TiledEqualsSerial：多三角形重叠场景（含跨 tile、半透明、深度竞争）tiled 与 serial 输出 colorData+depthData 逐位一致
2. ThreadsClampToTiles：threads=0/超大值不崩溃、结果一致
