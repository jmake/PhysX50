using UnityEngine;

public class ObjectSelector : MonoBehaviour
{
    private GameObject currentlySelected;
    private Material originalMaterial;

    [SerializeField] private Color highlightColor = Color.blue;

    void Update()
    {
        if (IsLeftMouseClick())
        {
            TrySelectObjectUnderMouse();
        }

        if (IsLeftMouseHeld() && currentlySelected != null)
        {
            MoveSelectedObjectOnXZPlane();
        }

        if (IsLeftMouseReleased())
        {
            ResetCurrentlySelected();
        }
    }

    bool IsLeftMouseClick()
    {
        return Input.GetMouseButtonDown(0);
    }

    bool IsLeftMouseHeld()
    {
        return Input.GetMouseButton(0);
    }

    bool IsLeftMouseReleased()
    {
        return Input.GetMouseButtonUp(0);
    }

    void TrySelectObjectUnderMouse()
    {
        Ray ray = GetMouseRay();
        if (Physics.Raycast(ray, out RaycastHit hit))
        {
            GameObject targetObject = hit.collider.gameObject;

            HighlightObject(targetObject);
            currentlySelected = targetObject;
        }
    }

    void MoveSelectedObjectOnXZPlane()
    {
        Plane xzPlane = new Plane(Vector3.up, currentlySelected.transform.position);
        Ray ray = GetMouseRay();

        if (xzPlane.Raycast(ray, out float distance))
        {
            Vector3 point = ray.GetPoint(distance);
            Vector3 newPosition = new Vector3(point.x, currentlySelected.transform.position.y, point.z);
            currentlySelected.transform.position = newPosition;
        }
    }

    Ray GetMouseRay()
    {
        return Camera.main.ScreenPointToRay(Input.mousePosition);
    }

    void HighlightObject(GameObject obj)
    {
        Renderer renderer = obj.GetComponent<Renderer>();
        if (renderer != null)
        {
            originalMaterial = renderer.material;
            Material highlightMat = new Material(originalMaterial);
            highlightMat.color = highlightColor;
            renderer.material = highlightMat;
        }
    }

    void ResetCurrentlySelected()
    {
        if (currentlySelected != null)
        {
            Renderer renderer = currentlySelected.GetComponent<Renderer>();
            if (renderer != null && originalMaterial != null)
            {
                renderer.material = originalMaterial;
            }

            currentlySelected = null;
            originalMaterial = null;
        }
    }
}
