module Main exposing (..)

import Html exposing (..)
import Html.Attributes exposing (..)
import Html.Events exposing (..)
import Http
import Task
import Model.User exposing (..)
import Model.Msg exposing (..)
import Api.User exposing (..)
import Component.User exposing (..)


-- main application model


type alias Model =
    { user : Maybe User
    , userId : Int
    , isLoading : Bool
    , error : Maybe Http.Error
    , swapCount : Int
    }



-- initializes the model and provides initial commands


init : ( Model, Cmd Msg )
init =
    let
        initialModel =
            { user = Nothing
            , userId = 0
            , isLoading = False
            , error = Nothing
            , swapCount = 0
            }
    in
        Debug.log "init"
            ( initialModel, Cmd.none )



-- action helpers


loadFirstUser : Cmd Msg
loadFirstUser =
    Task.perform identity (Task.succeed LoadUser)



-- renders the view based on model


view : Model -> Html Msg
view model =
    div []
        [ img [ src "assets/logo.png" ] []
        , h1 [] [ text "Elm HTTP request playground" ]
        , button [ onClick LoadUser ] [ text "Load next user" ]
        , viewUser model
        , em [] [ text ("parts of this application have been hot-swapped " ++ (toString model.swapCount) ++ " times") ]
        , p [] [ text (toString model) ]
        ]


viewUser : Model -> Html Msg
viewUser model =
    case model.error of
        Just error ->
            viewError error

        Nothing ->
            if model.isLoading then
                viewLoading model
            else
                viewUserInfo model.user


viewLoading : Model -> Html Msg
viewLoading model =
    div []
        [ h2 [] [ text "Loading user" ]
        , div [] [ text ("Loading user #" ++ (toString model.userId) ++ " info") ]
        ]


viewError : Http.Error -> Html Msg
viewError error =
    div [ class "error" ]
        [ h2 [] [ text "Error occured" ]
        , div [] [ text (getErrorMessage error) ]
        ]


getErrorMessage : Http.Error -> String
getErrorMessage error =
    case error of
        Http.Timeout ->
            "request timed out"

        Http.NetworkError ->
            "network error occured"

        Http.BadUrl url ->
            "invalid url '" ++ url ++ "' provided"

        Http.BadStatus response ->
            "request failed with " ++ (toString response.status.code) ++ " (" ++ response.status.message ++ ")"

        Http.BadPayload problem response ->
            "got invalid response (" ++ problem ++ ")"



-- updates the model based on given message


update : Msg -> Model -> ( Model, Cmd Msg )
update msg model =
    case msg of
        LoadUser ->
            let
                newUserId =
                    model.userId + 1
            in
                ( { model | userId = newUserId, isLoading = True, error = Nothing }, loadUser newUserId )

        UserResult (Ok loadedUser) ->
            ( { model | user = Just loadedUser, isLoading = False, error = Nothing }, Cmd.none )

        UserResult (Err error) ->
            -- todo: handle error
            ( { model | error = Just error, isLoading = False }, Cmd.none )



-- subscriptions


subscriptions : Model -> Sub Msg
subscriptions model =
    Sub.none



-- main program


main : Program Never Model Msg
main =
    Html.program
        { init = init
        , view = view
        , update = update
        , subscriptions = subscriptions
        }
