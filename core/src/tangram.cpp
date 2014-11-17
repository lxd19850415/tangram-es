#include "tangram.h"

#include <memory>
#include <utility>

#include "platform.h"
#include "tile/tileManager.h"
#include "view/view.h"
#include "data/dataSource.h"
#include "style/style.h"
#include "scene/scene.h"

namespace Tangram {

std::unique_ptr<TileManager> m_tileManager;    
std::shared_ptr<Scene> m_scene;
std::shared_ptr<View> m_view;

void initialize() {
    
    logMsg("%s\n", "initialize");

    // Create view
    m_view = std::make_shared<View>();

    // Move the view to coordinates in Manhattan so we have something interesting to test
    glm::dvec2 target = m_view->getMapProjection().LonLatToMeters(glm::dvec2(-74.00796, 40.70361));
    m_view->setPosition(target.x, target.y);

    // Load style(s); hard-coded for now
    std::unique_ptr<Style> polyStyle(new PolygonStyle("Polygon"));
    polyStyle->addLayers({
        "buildings",
        //"water",
        //"earth",
        //"landuse",
        "roads"
    });

    // Create a scene definition and add the style
    m_scene = std::make_shared<Scene>();
    m_scene->addStyle(std::move(polyStyle));

    // Create a tileManager
    m_tileManager = TileManager::GetInstance();
    
    // Pass references to the view and scene into the tile manager
    m_tileManager->setView(m_view);
    m_tileManager->setScene(m_scene);

    // Add a tile data source
    std::unique_ptr<DataSource> dataSource(new MapzenVectorTileJson());
    m_tileManager->addDataSource(std::move(dataSource));

    // Set up openGL state
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    logMsg("%s\n", "finish initialize");

}

void resize(int _newWidth, int _newHeight) {
    
    logMsg("%s\n", "resize");

    glViewport(0, 0, _newWidth, _newHeight);

    if (m_view) {
        m_view->setAspect(_newWidth, _newHeight);
    }

}

void update(float _dt) {

    if (m_tileManager) {
        m_tileManager->updateTileSet();
    }

}

void render() {

    // Set up openGL for new frame
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::dmat4 viewProj = m_view->getViewProjectionMatrix();

    // Loop over all styles
    for (const auto& style : m_scene->getStyles()) {

        style->setup();

        // Loop over visible tiles
        for (const auto& mapIDandTile : m_tileManager->getVisibleTiles()) {

            const std::unique_ptr<MapTile>& tile = mapIDandTile.second;
            
            if (tile) {
                // Draw!
                tile->draw(*style, viewProj);
            }

        }
    }

    // TODO: This error checking is incomplete and only marginally useful 
    // 1. We need to continue calling glGetError until no error states remain
    // 2. Repeating an error message 60 times per second is not useful, try to consolidate 
    GLenum glError = glGetError();
    if (glError) {
        logMsg("GL Error %d!!!\n", glError);
    }

}

void teardown() {
    // TODO: Release resources!
}
    
}
